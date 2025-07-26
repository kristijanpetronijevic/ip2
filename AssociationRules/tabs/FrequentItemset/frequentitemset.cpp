#include "frequentitemset.h"


FrequentItemset::FrequentItemset()
{
    QString projectRoot = QCoreApplication::applicationDirPath() + "/../";
    QString absoluteRootPath = QDir(projectRoot).absolutePath();

    _inputOpenFilePath = absoluteRootPath + "/AssociationRules/resources/Frequent Itemset/";
    _outputOpenFilePath = absoluteRootPath + "/AssociationRules/resources/Frequent Itemset/";
    _inputFilePath = absoluteRootPath + "/AssociationRules/resources/Frequent Itemset/input4_10000.txt";
    _outputFilePath = absoluteRootPath + "/AssociationRules/resources/Frequent Itemset/output.txt";

    _nodeRadius = 25;

    _editor = new QTextEdit(nullptr);
    _editor->setReadOnly(true);
    _editor->setWindowTitle("Frequent Itemset");
    _editor->resize(400, 800);
}


FrequentItemset::~FrequentItemset()
{
    delete _editor;
}


QString FrequentItemset::getInputFilePath()
{
    return _inputFilePath;
}


QString FrequentItemset::getOutputFilePath()
{
    return _outputFilePath;
}


QString FrequentItemset::onBrowseButtonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Select File",
        _inputOpenFilePath,
        "Text files (*.txt)"
    );

    if(filePath.isEmpty()) {
        return _inputFilePath;
    }

    _inputFilePath = filePath;

    return filePath;
}


QString FrequentItemset::onChangeButtonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Select File",
        _outputOpenFilePath,
        "Text files (*.txt)"
    );

    if(filePath.isEmpty()) {
        return _outputFilePath;
    }

    _outputFilePath = filePath;

    return filePath;
}


void FrequentItemset::onRunAlgorithmButtonClicked(QGraphicsScene *scene, const double minSupport)
{
    scene->clear();
    _transactions.clear();
    _sortedTransactions.clear();
    _itemsFrequencies.clear();
    _setsFrequencies.clear();
    _childrenMap.clear();
    _pendingRemovalEllipses.clear();
    _frequentItemsets.clear();
    _nodesSupport.clear();
    _nodePositions.clear();
    _removalColoring = true;

    bool readFileSuccess = readFile();
    if(!readFileSuccess) {
        return;
    }

    findItemFrequencies();
    sortTransactions();
    findSetsFrequencies();

    _minSupport = minSupport * _sortedTransactions.size();

    removeRareItemsets();
    findChildren();

    _currentChildrenMap = _childrenMap;

    drawTree(scene);
}


void FrequentItemset::onForwardButtonClicked(QGraphicsScene *scene)
{
    if(_childrenMap.size() == 0) {
        QMessageBox::critical(nullptr, "Error", "Run the algorithm first");
        return;
    }

    if(_currentChildrenMap.size() == 0) {
        QVector<QVector<int>> frequentItemsetVector = _frequentItemsets.keys();
        std::sort(frequentItemsetVector.begin(), frequentItemsetVector.end(),
            [](const QVector<int> &a, const QVector<int> &b) {
                if(a.size() == b.size()) {
                    return a < b;
                }

                return a.size() < b.size();
            }
        );

        saveFile(frequentItemsetVector);

        return;
    }

    int maxDepth = 0;
    for(auto it = _currentChildrenMap.cbegin(); it != _currentChildrenMap.cend(); it++) {
        int keySize = it.key().size();
        maxDepth = std::max(maxDepth, keySize);
    }

    QVector<QVector<int>> deepestNodes;
    for(auto it = _currentChildrenMap.cbegin(); it != _currentChildrenMap.cend(); it++) {
        if(it.key().size() == maxDepth) {
            deepestNodes.append(it.key());
        }
    }

    QVector<int> nodeToRemove;
    int minFrequency = INT_MAX;
    for(auto it = deepestNodes.cbegin(); it != deepestNodes.cend(); it++) {
        int frequency = _itemsFrequencies.value(it->last(), INT_MAX);
        if(frequency < minFrequency) {
            minFrequency = frequency;
            nodeToRemove = *it;
        }
    }

    QVector<QVector<int>> nodesToRemove;
    for(auto it = _currentChildrenMap.cbegin(); it != _currentChildrenMap.cend(); it++) {
        if(it.key().last() == nodeToRemove.last() && it.value().isEmpty()) {
            nodesToRemove.append(it.key());
        }
    }

    QString message = "Paths:\n";
    QMap<QVector<int>, int> paths;
    for(QVector<int> &node : nodesToRemove) {
        paths[node] = _nodesSupport[node];

        for(int i : node) {
            message += QString::number(i);
        }
        message += "\n";
    }

    QVector<QVector<int>> candidates;
    int lastElement = nodeToRemove.last();
    QVector<QVector<int>> nodesToRemoveCopy = nodesToRemove;
    message += "\nCandidates:\n";
    for(int i = 0; i < nodesToRemoveCopy.size(); i++) {
        nodesToRemoveCopy[i].pop_back();

        int nodeSize = nodesToRemoveCopy[i].size();
        int numCombinations = pow(2, nodeSize);
        for(int j = 0; j < numCombinations; j++) {
            QVector<int> combination;
            for(int k = 0; k < nodeSize; k++) {
                if(j & (1 << k)) {
                    combination.push_back(nodesToRemoveCopy[i][k]);
                }
            }

            combination.push_back(lastElement);

            auto foundIt = std::find(candidates.begin(), candidates.end(), combination);
            if(foundIt == candidates.end()) {
                candidates.push_back(combination);
                QVector<int> sortedCombination = combination;
                std::sort(sortedCombination.begin(), sortedCombination.end());
                message += "{";
                for(int i : combination) {
                    message += QString::number(i) + ", ";
                }
                message = message.removeLast().removeLast() + "}  #SUP: " + QString::number(_setsFrequencies[sortedCombination]) + "\n";
            }
        }
    }

    message += "\nFrequent itemsets:\n";
    for(QVector<int> &set : candidates) {
        QVector<int> sortedSet = set;
        std::sort(sortedSet.begin(), sortedSet.end());
        auto foundIt = _frequentItemsets.find(sortedSet);
        if((_minSupport - _setsFrequencies[sortedSet] < 0.001) && foundIt == _frequentItemsets.end()) {
            message += "{";
            for(int i : set) {
                message += QString::number(i) + ", ";
            }
            message = message.removeLast().removeLast() + "}  #SUP: " + QString::number(_setsFrequencies[sortedSet]) + "\n";
            _frequentItemsets[sortedSet] = _setsFrequencies[sortedSet];
        }
    }

    if(_removalColoring) {
        for(const auto &node : nodesToRemove) {
            QPointF pos = _nodePositions[node];
            QGraphicsEllipseItem *nodeToRemove = nullptr;
            QGraphicsTextItem *textToRemove = nullptr;
            QList<QGraphicsItem*> itemsInArea = scene->items(
                QRectF(
                    pos.x() - _nodeRadius, pos.y() - _nodeRadius,
                    _nodeRadius * 2, _nodeRadius * 2
                )
            );

            for(QGraphicsItem *item : itemsInArea) {
                if(auto ellipse = dynamic_cast<QGraphicsEllipseItem*>(item)) {
                    nodeToRemove = ellipse;
                } else if(auto text = dynamic_cast<QGraphicsTextItem*>(item)) {
                    textToRemove = text;
                }
            }

            if(nodeToRemove) {
                scene->removeItem(nodeToRemove);
                delete nodeToRemove;
            }

            if(textToRemove) {
                scene->removeItem(textToRemove);
                delete textToRemove;
            }

            QGraphicsEllipseItem *ellipse = scene->addEllipse(
                pos.x() - _nodeRadius, pos.y() - _nodeRadius,
                _nodeRadius * 2, _nodeRadius * 2,
                QPen(Qt::black), QBrush(Qt::red)
            );
            QString nodeText = QString::number(node.last()) + ": " + QString::number(_nodesSupport[node]);
            QGraphicsTextItem *text = scene->addText(nodeText);
            QFont font = text->font();
            font.setPointSize(7);
            text->setFont(font);
            text->setPos(
                pos.x() - text->boundingRect().width() / 2,
                pos.y() - text->boundingRect().height() / 2
            );
            text->setDefaultTextColor(Qt::black);
            _pendingRemovalEllipses.append(ellipse);
        }

        _editor->setText(message);
        _editor->show();

        _removalColoring = false;
    } else {
        for(const auto &node : nodesToRemove) {
            for(auto &parent : _currentChildrenMap.keys()) {
                if(_currentChildrenMap[parent].contains(node)) {
                    _currentChildrenMap[parent].removeOne(node);
                    break;
                }
            }
            _currentChildrenMap.remove(node);
        }

        for(auto &ellipse : _pendingRemovalEllipses) {
            QList<QGraphicsItem*> items = scene->items(
                QRectF(
                    ellipse->rect().center().x() - _nodeRadius,
                    ellipse->rect().center().y() - _nodeRadius,
                    _nodeRadius * 2, _nodeRadius * 2
                )
            );

            for(QGraphicsItem *item : items) {
                if(auto *line = dynamic_cast<QGraphicsLineItem*>(item)) {
                    scene->removeItem(line);
                    delete line;
                } else if(auto *text = dynamic_cast<QGraphicsTextItem*>(item)) {
                    scene->removeItem(text);
                    delete text;
                }
            }

            scene->removeItem(ellipse);
            delete ellipse;
        }

        _pendingRemovalEllipses.clear();
        _removalColoring = true;
    }
}


bool FrequentItemset::readFile()
{
    QFile file(_inputFilePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Error", "Unable to open the input file");
        return false;
    }

    QTextStream in(&file);
    while(!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if(line.isEmpty()) {
            QMessageBox::critical(nullptr, "Error", "Invalid input file: Empty line");
            return false;
        }

        QStringList items = line.split(" ", Qt::SkipEmptyParts);
        QVector<int> transaction;
        for(const QString &item : items) {
            bool isNumber = false;
            int itemInt = item.toInt(&isNumber);

            if(!isNumber) {
                QMessageBox::critical(nullptr, "Error", "Invalid input file: Non-numeric value");
                return false;
            }

            transaction.append(itemInt);
        }

        std::sort(transaction.begin(), transaction.end());
        _transactions.append(transaction);
    }

    file.close();

    return true;
}


void FrequentItemset::findItemFrequencies()
{
    for(const QVector<int> &transaction : _transactions) {
        for(int item : transaction) {
            _itemsFrequencies[item]++;
        }
    }
}


void FrequentItemset::sortTransactions()
{
    _sortedTransactions = _transactions;
    for(QVector<int> &transaction : _sortedTransactions) {
        std::sort(transaction.begin(), transaction.end(),
            [this](int a, int b) {
                return _itemsFrequencies.value(a, 0) > _itemsFrequencies.value(b, 0);
            }
        );
    }
}


void FrequentItemset::findSetsFrequencies()
{
    QVector<int> items = _itemsFrequencies.keys();
    int numItems = items.size();
    int totalCombinations = pow(2, numItems);

    for(int mask = 1; mask < totalCombinations; mask++) {
        QVector<int> combination;

        for(int i = 0; i < numItems; i++) {
            if(mask & (1 << i)) {
                combination.append(items[i]);
            }
        }

        int count = 0;
        for(const QVector<int> &transaction : _transactions) {
            bool isSubset = std::all_of(
                combination.begin(), combination.end(),
                [&transaction](int item) {
                    return transaction.contains(item);
                }
            );

            if(isSubset) {
                count++;
            }
        }

        _setsFrequencies[combination] = count;
    }
}


void FrequentItemset::removeRareItemsets()
{
    for(QVector<int> &transaction : _sortedTransactions) {
        auto it = transaction.begin();
        while(it != transaction.end()) {
            if(_minSupport - _itemsFrequencies[*it] > 0.001) {
                it = transaction.erase(it);
            } else {
                it++;
            }
        }
    }
}


void FrequentItemset::findChildren()
{
    for(QVector<int> &transaction : _sortedTransactions) {
        QVector<int> current;

        for(int item : transaction) {
            current.push_back(item);

            if(_childrenMap.find(current) == _childrenMap.end()) {
                _childrenMap[current] = {};
            }

            if(current.size() < transaction.size()) {
                QVector<int> child = current;
                child.push_back(transaction[current.size()]);

                if(!childExists(_childrenMap[current], child)) {
                    _childrenMap[current].push_back(child);
                }
            }
        }
    }
}


bool FrequentItemset::childExists(const QVector<QVector<int>> &childrenList, const QVector<int> &child)
{
    for(auto &existingChild : childrenList) {
        if(existingChild == child) {
            return true;
        }
    }

    return false;
}


void FrequentItemset::drawTree(QGraphicsScene *scene)
{
    QMap<int, int> widths;
    for(auto it = _childrenMap.begin(); it != _childrenMap.end(); it++) {
        QVector<int> node = it.key();
        int keySize = node.size();
        widths[keySize]++;
        _nodesSupport[node] = 0;

        for(const QVector<int> &child : it.value()) {
            bool existsInKeys = false;
            int valueSize = child.size();

            for(auto keyIt = _childrenMap.begin(); keyIt != _childrenMap.end(); keyIt++) {
                if(keyIt.key() == child) {
                    existsInKeys = true;
                    break;
                }
            }

            if(!existsInKeys) {
                widths[valueSize]++;
                _nodesSupport[child] = 0;
            }
        }
    }

    int maxWidth = 0;
    for(auto it = widths.begin(); it != widths.end(); it++) {
        maxWidth = std::max(maxWidth, it.value());
    }

    for(QVector<int> &transaction : _sortedTransactions) {
        QVector<int> current;
        for(int item : transaction) {
            current.push_back(item);
            _nodesSupport[current]++;
        }
    }

    maxWidth *= 100;
    int levelHeight = 130;
    int currentY = 0;

    QPointF rootPos(maxWidth / 2.0, currentY);
    scene->addEllipse(
        rootPos.x() - _nodeRadius, rootPos.y() - _nodeRadius,
        _nodeRadius * 2, _nodeRadius * 2,
        QPen(Qt::black), QBrush(Qt::white)
    );
    QGraphicsTextItem *rootText = scene->addText("null");
    QFont font = rootText->font();
    font.setPointSize(7);
    rootText->setFont(font);
    rootText->setPos(
        rootPos.x() - rootText->boundingRect().width() / 2,
        rootPos.y() - rootText->boundingRect().height() / 2
    );
    rootText->setDefaultTextColor(Qt::black);

    currentY += levelHeight;

    for(auto it = widths.begin(); it != widths.end(); it++) {
        int level = it.key();
        int nodeCount = it.value();

        QVector<QPointF> positions;

        for(int i = 0; i < nodeCount; i++) {
            double xPosition = (maxWidth / static_cast<double>(nodeCount)) * (i + 0.5);
            positions.append(QPointF(xPosition, currentY));
        }

        int index = 0;
        for(auto it = _childrenMap.begin(); it != _childrenMap.end(); it++) {
            const QVector<int> &childKey = it.key();
            if(childKey.size() == level) {
                _nodePositions[childKey] = positions[index++];
                if(index >= positions.size()) {
                    break;
                }
            }
        }

        for(auto it = _childrenMap.begin(); it != _childrenMap.end(); it++) {
            const QVector<int> &childKey = it.key();
            if(childKey.size() == level) {
                QPointF pos = _nodePositions[childKey];
                scene->addEllipse(
                    pos.x() - _nodeRadius, pos.y() - _nodeRadius,
                    _nodeRadius * 2, _nodeRadius * 2,
                    QPen(Qt::black), QBrush(Qt::white)
                );

                QString nodeText = QString::number(childKey.last()) + ": " + QString::number(_nodesSupport[childKey]);
                QGraphicsTextItem *text = scene->addText(nodeText);
                QFont font = text->font();
                font.setPointSize(7);
                text->setFont(font);
                text->setPos(
                    pos.x() - text->boundingRect().width() / 2,
                    pos.y() - text->boundingRect().height() / 2
                );
                text->setDefaultTextColor(Qt::black);
            }
        }

        currentY += levelHeight;
    }

    for(auto it = _childrenMap.begin(); it != _childrenMap.end(); it++) {
        const QVector<int> &childKey = it.key();
        if(childKey.size() == 1) {
            QPointF childPos = _nodePositions[childKey];
            scene->addLine(
                rootPos.x(), rootPos.y() + _nodeRadius,
                childPos.x(), childPos.y() - _nodeRadius,
                QPen(Qt::white)
            );
        }
    }

    for(auto it = _childrenMap.begin(); it != _childrenMap.end(); it++) {
        const QVector<int> &parent = it.key();
        const QVector<QVector<int>> &children = it.value();

        if(!_nodePositions.contains(parent)) {
            continue;
        }

        QPointF parentPos = _nodePositions[parent];
        for(const QVector<int> &child : children) {
            if(_nodePositions.contains(child)) {
                QPointF childPos = _nodePositions[child];
                scene->addLine(
                    parentPos.x(), parentPos.y() + _nodeRadius,
                    childPos.x(), childPos.y() - _nodeRadius,
                    QPen(Qt::white)
                );
            }
        }
    }
}


void FrequentItemset::saveFile(const QVector<QVector<int>> &frequentItemsets)
{
    QFile file(_outputFilePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Error", "Unable to open the output file");
        return;
    }

    QTextStream out(&file);
    QString message = "";
    const int itemsetWidth = 20;
    const int supportWidth = 30;

    for(QVector<int> set : frequentItemsets) {
        int support = _frequentItemsets[set];
        double supportPerc = static_cast<double>(support) / _sortedTransactions.size() * 100;
        message = "{";
        for(int num : set) {
            message += QString::number(num) + ", ";
        }
        message = message.removeLast().removeLast() + "}";

        out << message.leftJustified(itemsetWidth)
            << QString("#SUP: %1 (%2%)")
               .arg(support)
               .arg(supportPerc)
               .leftJustified(supportWidth)
            << "\n";
    }

    file.close();

    if(!QDesktopServices::openUrl(QUrl::fromLocalFile(_outputFilePath))) {
        QMessageBox::critical(nullptr, "Error", "Unable to read from output file");
    }
}
