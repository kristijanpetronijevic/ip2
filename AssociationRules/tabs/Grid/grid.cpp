#include "grid.h"


Grid::Grid()
{
    QString projectRoot = QCoreApplication::applicationDirPath() + "/../";
    QString absoluteRootPath = QDir(projectRoot).absolutePath();

    _inputOpenFilePath = absoluteRootPath + "/AssociationRules/resources/Grid/";
    _outputOpenFilePath = absoluteRootPath + "/AssociationRules/resources/Grid/";
    _inputFilePath = absoluteRootPath + "/AssociationRules/resources/Grid/input4_100000.txt";
    _outputFilePath = absoluteRootPath + "/AssociationRules/resources/Grid/output.txt";

    _gridWidth = 0;

    _closedAndMaximalItemsetsBrush = QBrush(Qt::cyan, Qt::SolidPattern);
    _maximalItemsetsBrush = QBrush(Qt::green, Qt::SolidPattern);
    _closedItemsetsBrush = QBrush(Qt::yellow, Qt::SolidPattern);
    _frequentItemsetsBrush = QBrush(Qt::white, Qt::SolidPattern);
    _rareItemsetsBrush = QBrush(Qt::gray, Qt::SolidPattern);
    _textPen = QPen(Qt::black, 2);
}


QString Grid::getInputFilePath()
{
    return _inputFilePath;
}


QString Grid::getOutputFilePath()
{
    return _outputFilePath;
}


QString Grid::onBrowseButtonClicked()
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


QString Grid::onChangeButtonClicked()
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


void Grid::onRunAlgorithmButtonClicked(QGraphicsScene *scene, const double minSupport)
{
    _transactions.clear();
    scene->clear();
    QSet<int> gridItems;

    bool readFileSuccess = readFile(gridItems);
    if(!readFileSuccess) {
        return;
    }

    QVector<QVector<int>> gridSets = findAllSets(gridItems);
    QVector<int> gridSupports = findAllSupports(gridSets);
    QMap<QVector<int>, QVector<QVector<int>>> childrenMap = findChildren(gridSets);
    QMap<QVector<int>, int> frequentItemsets = generateFrequentItemsets(minSupport);
    QVector<QVector<int>> frequentKeys = frequentItemsets.keys().toVector();
    QVector<QVector<int>> closedItemsets = findClosedItemsets(frequentItemsets);
    QVector<QVector<int>> maximalItemsets = findMaximalItemsets(frequentItemsets);
    QVector<QVector<int>> closedAndMaximalItemsets = findClosedAndMaximalItemsets(closedItemsets, maximalItemsets);

    drawGrid(scene, gridSets, childrenMap, gridSupports, frequentKeys, closedItemsets, maximalItemsets, closedAndMaximalItemsets);

    QBrush legendBrush = QBrush(Qt::white, Qt::SolidPattern);
    QGraphicsRectItem *legendRect = scene->addRect(10 - (_gridWidth / 2), 10, 230, 65, _textPen, legendBrush);
    QGraphicsEllipseItem *closedItemsetsEllipse = scene->addEllipse(15 - (_gridWidth / 2), 15, 15, 15, _textPen, _closedItemsetsBrush);
    QGraphicsTextItem *closedItemsetsText = scene->addText("Closed itemsets");
    closedItemsetsText->setPos(30 - (_gridWidth / 2), 10);
    closedItemsetsText->setDefaultTextColor(Qt::black);
    QGraphicsEllipseItem *maximalItemsetsEllipse = scene->addEllipse(15 - (_gridWidth / 2), 35, 15, 15, _textPen, _maximalItemsetsBrush);
    QGraphicsTextItem *maximalItemsetsText = scene->addText("Maximal itemsets");
    maximalItemsetsText->setPos(30 - (_gridWidth / 2), 30);
    maximalItemsetsText->setDefaultTextColor(Qt::black);
    QGraphicsEllipseItem *closedAndMaximalItemsetsEllipse = scene->addEllipse(15 - (_gridWidth / 2), 55, 15, 15, _textPen, _closedAndMaximalItemsetsBrush);
    QGraphicsTextItem *closedAndMaximalItemsetsText = scene->addText("Closed and maximal itemsets");
    closedAndMaximalItemsetsText->setPos(30 - (_gridWidth / 2), 50);
    closedAndMaximalItemsetsText->setDefaultTextColor(Qt::black);

    QVector<QPair<QVector<int>, int>> sortedItemsets = sortBySetSize(frequentItemsets);
    bool saveFileSuccess = saveFile(sortedItemsets, closedItemsets, maximalItemsets, closedAndMaximalItemsets);
    if(!saveFileSuccess) {
        return;
    }

    _gridWidth = 0;
}


bool Grid::readFile(QSet<int> &gridItems)
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

            if(!gridItems.contains(itemInt)) {
                gridItems.insert(itemInt);
            }
            transaction.append(itemInt);
        }

        std::sort(transaction.begin(), transaction.end());
        _transactions.append(transaction);
    }

    file.close();

    return true;
}


QVector<QVector<int>> Grid::findAllSets(const QSet<int> &gridItems)
{
    QVector<QVector<int>> allSets;
    allSets.append(QVector<int>());
    QVector<int> items = QVector<int>(gridItems.begin(), gridItems.end());

    for(int item : items) {
        int currentSize = allSets.size();

        for(int i = 0; i < currentSize; i++) {
            QVector<int> newSubset = allSets[i];
            newSubset.append(item);
            std::sort(newSubset.begin(), newSubset.end());
            allSets.append(newSubset);
        }
    }

    std::sort(allSets.begin(), allSets.end(),
        [](const QVector<int> &a, const QVector<int> &b) {
            if(a.size() == b.size()) {
                return a < b;
            }
            return a.size() < b.size();
        }
    );

    return allSets;
}


QVector<int> Grid::findAllSupports(const QVector<QVector<int>> &gridSets)
{
    QVector<int> supports;
    for(const QVector<int> &set : gridSets) {
        int count = 0;
        for(const QVector<int> &transaction : _transactions) {
            bool contains = true;
            for(int item : set) {
                if(!transaction.contains(item)) {
                    contains = false;
                    break;
                }
            }

            if(contains) {
                count++;
            }
        }

        int support = count;
        supports.append(support);
    }

    return supports;
}


QMap<QVector<int>, QVector<QVector<int>>> Grid::findChildren(QVector<QVector<int>> &gridSets)
{
    QMap<QVector<int>, QVector<QVector<int>>> childrenMap;

    for(const QVector<int> &parent : gridSets) {
        QVector<QVector<int>> children;

        for(const QVector<int> &potentialChild : gridSets) {
            if(parent.size() + 1 == potentialChild.size() &&
                std::includes(potentialChild.begin(), potentialChild.end(), parent.begin(), parent.end())
            ) {
                children.append(potentialChild);
            }
        }

        childrenMap[parent] = children;
    }

    return childrenMap;
}


QMap<QVector<int>, int> Grid::generateFrequentItemsets(const double minSupport)
{
    QMap<QVector<int>, int> frequentItemsets;

    for(QVector<int> &transaction : _transactions) {
        QVector<QVector<int>> subsets = generateSubsets(transaction);
        for(const QVector<int> &subset : subsets) {
            frequentItemsets[subset]++;
        }
    }

    double transactionsSize = _transactions.size();
    for(auto it = frequentItemsets.begin(); it != frequentItemsets.end(); ) {
        if((it.value() / transactionsSize) < minSupport) {
            it = frequentItemsets.erase(it);
        } else {
            it++;
        }
    }

    return frequentItemsets;
}


QVector<QVector<int>> Grid::findClosedItemsets(const QMap<QVector<int>, int> &frequentItemsets)
{
    QVector<QVector<int>> closedItemsets;

    for(auto it1 = frequentItemsets.begin(); it1 != frequentItemsets.end(); it1++) {
        bool isClosed = true;
        for(auto it2 = frequentItemsets.begin(); it2 != frequentItemsets.end(); it2++) {
            auto it1Begin = it1.key().begin();
            auto it1End = it1.key().end();
            auto it2Begin = it2.key().begin();
            auto it2End = it2.key().end();
            if(it1.key() != it2.key() && std::includes(it2Begin, it2End, it1Begin, it1End) && it1.value() == it2.value()) {
                isClosed = false;
                break;
            }
        }

        if(isClosed) {
            closedItemsets.append(it1.key());
        }
    }

    return closedItemsets;
}


QVector<QVector<int>> Grid::findMaximalItemsets(const QMap<QVector<int>, int> &frequentItemsets)
{
    QVector<QVector<int>> maximalItemsets;

    for(auto it1 = frequentItemsets.begin(); it1 != frequentItemsets.end(); it1++) {
        bool isMaximal = true;
        for(auto it2 = frequentItemsets.begin(); it2 != frequentItemsets.end(); it2++) {
            auto it1Begin = it1.key().begin();
            auto it1End = it1.key().end();
            auto it2Begin = it2.key().begin();
            auto it2End = it2.key().end();
            if(it1.key() != it2.key() && std::includes(it2Begin, it2End, it1Begin, it1End)) {
                isMaximal = false;
                break;
            }
        }

        if(isMaximal) {
            maximalItemsets.append(it1.key());
        }
    }

    return maximalItemsets;
}


QVector<QVector<int>> Grid::findClosedAndMaximalItemsets(
    const QVector<QVector<int>> &closedItemsets,
    const QVector<QVector<int>> &maximalItemsets
)
{
    QVector<QVector<int>> closedAndMaximalItemsets;

    for(const auto &closedItemset : closedItemsets) {
        auto found = std::find(maximalItemsets.begin(), maximalItemsets.end(), closedItemset);
        if(found != maximalItemsets.end()) {
            closedAndMaximalItemsets.append(closedItemset);
        }
    }

    return closedAndMaximalItemsets;
}


void Grid::drawGrid(
    QGraphicsScene *scene,
    const QVector<QVector<int>> &gridSets,
    const QMap<QVector<int>, QVector<QVector<int>>> &childrenMap,
    const QVector<int> &gridSupports,
    const QVector<QVector<int>> &frequentKeys,
    const QVector<QVector<int>> &closedItemsets,
    const QVector<QVector<int>> &maximalItemsets,
    const QVector<QVector<int>> &maximalAndClosedItemsets
)
{
    QMap<QVector<int>, QPointF> nodePositions;

    const int nodeRadius = 25;
    const int verticalSpacing = 130;
    const int horizontalSpacing = 100;

    QMap<int, QVector<QVector<int>>> levels;
    for(const QVector<int> &set : gridSets) {
        levels[set.size()].append(set);
    }

    int y = 50;
    int index = 0;

    for(auto it = levels.begin(); it != levels.end(); it++) {
        const QVector<QVector<int>> &levelSets = it.value();
        _gridWidth = std::max(_gridWidth, static_cast<int>(levelSets.size()) * horizontalSpacing);
        int x = -(levelSets.size() - 1) * horizontalSpacing / 2;

        for(const QVector<int> &set : levelSets) {
            QGraphicsEllipseItem* node = scene->addEllipse(
                x - nodeRadius, y - nodeRadius,
                nodeRadius * 2, nodeRadius * 2,
                QPen(Qt::black), _frequentItemsetsBrush
            );

            QString setStr = "";
            for(int i = 0; i < set.size(); i++) {
                setStr += QString::number(set[i]);
                if(i < set.size() - 1) {
                    setStr += ", ";
                }
            }

            if(index == 0) {
                QGraphicsTextItem *rootText = scene->addText("null");
                QFont font = rootText->font();
                font.setPointSize(7);
                rootText->setFont(font);
                rootText->setPos(x - rootText->boundingRect().width() / 2, y - rootText->boundingRect().height() / 2);
                rootText->setDefaultTextColor(Qt::black);
            } else {
                auto foundMaxAndClosed = std::find(maximalAndClosedItemsets.begin(), maximalAndClosedItemsets.end(), set);
                auto foundMax = std::find(maximalItemsets.begin(), maximalItemsets.end(), set);
                auto foundClosed = std::find(closedItemsets.begin(), closedItemsets.end(), set);
                auto foundFrequent = std::find(frequentKeys.begin(), frequentKeys.end(), set);

                if(foundMaxAndClosed != maximalAndClosedItemsets.end()) {
                    node->setBrush(_closedAndMaximalItemsetsBrush);
                } else if(foundMax != maximalItemsets.end()) {
                    node->setBrush(_maximalItemsetsBrush);
                } else if(foundClosed != closedItemsets.end()) {
                    node->setBrush(_closedItemsetsBrush);
                } else if(foundFrequent != frequentKeys.end()) {
                    node->setBrush(_frequentItemsetsBrush);
                } else {
                    node->setBrush(_rareItemsetsBrush);
                }

                QGraphicsTextItem *setText = scene->addText(setStr);
                QFont font = setText->font();
                font.setPointSize(7);
                setText->setFont(font);
                setText->setPos(x - setText->boundingRect().width() / 2, y - setText->boundingRect().height() / 2 - 5);
                setText->setDefaultTextColor(Qt::black);

                double supportPerc = (gridSupports[index] / static_cast<double>(_transactions.size())) * 100;
                QString supportStr = "(" + QString::number(supportPerc) + " %)";
                QGraphicsTextItem *supportText = scene->addText(supportStr);
                supportText->setFont(font);
                supportText->setPos(x - supportText->boundingRect().width() / 2, y - supportText->boundingRect().height() / 2 + 5);
                supportText->setDefaultTextColor(Qt::black);
            }

            QPointF position(x, y);
            nodePositions[set] = position;

            x += horizontalSpacing;
            index++;
        }
        y += verticalSpacing;
    }

    for(auto it = childrenMap.begin(); it != childrenMap.end(); it++) {
        const QVector<int> &parent = it.key();
        const QVector<QVector<int>> &children = it.value();

        QPointF parentPos = nodePositions[parent];
        for(const QVector<int> &child : children) {
            QPointF childPos = nodePositions[child];

            QPointF lineStart(parentPos.x(), parentPos.y() + nodeRadius);
            QPointF lineEnd(childPos.x(), childPos.y() - nodeRadius);

            scene->addLine(lineStart.x(), lineStart.y(), lineEnd.x(), lineEnd.y(), QPen(Qt::white));
        }
    }

    scene->setSceneRect(-_gridWidth / 2, 0, _gridWidth, y);
}


bool Grid::saveFile(
    const QVector<QPair<QVector<int>, int>> &frequentItemsets,
    const QVector<QVector<int>> &closedItemsets,
    const QVector<QVector<int>> &maximalItemsets,
    const QVector<QVector<int>> &closedAndMaximalItemsets
)
{
    QFile file(_outputFilePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Error", "Unable to open the output file");
        return false;
    }

    const int itemsetWidth = 20;
    const int supportWidth = 30;
    const int categoryWidth = 20;

    QTextStream out(&file);
    for(const auto &pair : frequentItemsets) {
        const QVector<int> &itemset = pair.first;
        if(itemset.size() == 0)
            continue;

        int support = pair.second;
        double supportPerc = static_cast<double>(support) / _transactions.size() * 100;

        QString category;
        if (closedAndMaximalItemsets.contains(itemset)) {
            category = "Closed and Maximal";
        } else if (maximalItemsets.contains(itemset)) {
            category = "Maximal";
        } else if (closedItemsets.contains(itemset)) {
            category = "Closed";
        } else {
            category = "Frequent";
        }

        QString itemsetString = "{";
        for(int item : itemset) {
            itemsetString += QString::number(item) + ", ";
        }
        itemsetString = itemsetString.removeLast().removeLast();
        itemsetString += "}";

        out << itemsetString.leftJustified(itemsetWidth)
            << QString("#SUP: %1 (%2%)")
               .arg(support)
               .arg(supportPerc)
               .leftJustified(supportWidth)
            << category.leftJustified(categoryWidth) << "\n";
    }

    file.close();

    if(!QDesktopServices::openUrl(QUrl::fromLocalFile(_outputFilePath))) {
        QMessageBox::critical(nullptr, "Error", "Unable to read from output file");
        return false;
    }

    return true;
}


QVector<QVector<int>> Grid::generateSubsets(const QVector<int> &set)
{
    QVector<QVector<int>> subsets;
    int n = set.size();
    int totalSubsets = pow(2, n);

    for(int i = 0; i < totalSubsets; i++) {
        QVector<int> subset;
        for(int j = 0; j < n; j++) {
            if(i & (1 << j)) {
                subset.append(set[j]);
            }
        }

        subsets.append(subset);
    }

    return subsets;
}


QVector<QPair<QVector<int>, int>> Grid::sortBySetSize(const QMap<QVector<int>, int> &frequentItemsets) 
{
    QVector<QPair<QVector<int>, int>> itemList;

    for(auto it = frequentItemsets.cbegin(); it != frequentItemsets.cend(); it++) {
        itemList.append(qMakePair(it.key(), it.value()));
    }

    std::sort(itemList.begin(), itemList.end(), 
        [](const QPair<QVector<int>, int> &a, const QPair<QVector<int>, int> &b) {
            if(a.first.size() == b.first.size()) {
                return a.first < b.first;
            }

            return a.first.size() < b.first.size();
        }
    );

    return itemList;
}
