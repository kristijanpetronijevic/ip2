#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <cmath>
#include <stdexcept>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Association Rules");

    _gridTab = new Grid();
    MainWindow::gridConfig();
    _frequentItemsetTab = new FrequentItemset();
    MainWindow::frequentItemsetConfig();

    connect(ui->gridBrowseButton, &QPushButton::clicked, this, &MainWindow::gridOnBrowseButtonClicked);
    connect(ui->gridChangeButton, &QPushButton::clicked, this, &MainWindow::gridOnChangeButtonClicked);
    connect(ui->gridRunAlgorithmButton, &QPushButton::clicked, this, &MainWindow::gridOnRunAlgorithmButtonClicked);

    connect(ui->freqBrowseButton, &QPushButton::clicked, this, &MainWindow::freqOnBrowseButtonClicked);
    connect(ui->freqChangeButton, &QPushButton::clicked, this, &MainWindow::freqOnChangeButtonClicked);
    connect(ui->freqRunAlgorithmButton, &QPushButton::clicked, this, &MainWindow::freqOnRunAlgorithmButtonClicked);
    connect(ui->freqForwardButton, &QPushButton::clicked, this, &MainWindow::freqOnForwardButtonClicked);
    connect(ui->pbChooseVec1, &QPushButton::clicked, this, &MainWindow::pbChooseVector1);
    connect(ui->pbChooseVec2, &QPushButton::clicked, this, &MainWindow::pbChooseVector2);
    connect(ui->pbCompute, &QPushButton::clicked, this, &MainWindow::pbCompute);
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &MainWindow::changeParams);
    connect(ui->pbChooseApr, &QPushButton::clicked, this, &MainWindow::pbChooseApr);
    connect(ui->pbFindRare, &QPushButton::clicked, this, &MainWindow::pbFindRare);
    connect(ui->pbChooseOutput, &QPushButton::clicked, this, &MainWindow::pbChooseOutput);
}


void MainWindow::gridConfig()
{
    QString inputFilePath = _gridTab->getInputFilePath();
    QString outputFilePath = _gridTab->getOutputFilePath();
    ui->gridInputFileLine->setText(inputFilePath);
    ui->gridOutputFileLine->setText(outputFilePath);

    QTableWidgetItem *headerItem1 = new QTableWidgetItem(QString("Parameter"), QTableWidgetItem::Type);
    ui->gridParametersTable->setHorizontalHeaderItem(0, headerItem1);

    QTableWidgetItem *headerItem2 = new QTableWidgetItem(QString("Value"), QTableWidgetItem::Type);
    ui->gridParametersTable->setHorizontalHeaderItem(1, headerItem2);

    QTableWidgetItem *headerItem3 = new QTableWidgetItem(QString("Example"), QTableWidgetItem::Type);
    ui->gridParametersTable->setHorizontalHeaderItem(2, headerItem3);

    QTableWidgetItem *headerItem4 = new QTableWidgetItem(QString(""), QTableWidgetItem::Type);
    ui->gridParametersTable->setItem(0, 1, headerItem4);

    QTableWidgetItem *parameterCell = ui->gridParametersTable->item(0, 0);
    if(parameterCell) {
        parameterCell->setFlags(parameterCell->flags() & ~Qt::ItemIsEditable);
    } else {
        QMessageBox::critical(this, "Error", "Problem with getting parameter cell");
    }

    QTableWidgetItem *exampleCell = ui->gridParametersTable->item(0, 2);
    if(exampleCell) {
        exampleCell->setFlags(exampleCell->flags() & ~Qt::ItemIsEditable);
    } else {
        QMessageBox::critical(this, "Error", "Problem with getting example cell");
    }

    ui->gridParametersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    int rowCount = ui->gridParametersTable->rowCount();
    int columnCount = ui->gridParametersTable->columnCount();

    for(int row = 0; row < rowCount; row++) {
        for(int col = 0; col < columnCount; col++) {
            QTableWidgetItem *item = ui->gridParametersTable->item(row, col);
            if(item) {
                item->setTextAlignment(Qt::AlignCenter);
            }
        }
    }

    _gridScene = new QGraphicsScene();
    ui->gridGraphicsView->setScene(_gridScene);
    ui->gridGraphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
}


void MainWindow::gridOnBrowseButtonClicked()
{
    QString filePath = _gridTab->onBrowseButtonClicked();
    ui->gridInputFileLine->setText(filePath);
}


void MainWindow::gridOnChangeButtonClicked()
{
    QString filePath = _gridTab->onChangeButtonClicked();
    ui->gridOutputFileLine->setText(filePath);
}


void MainWindow::gridOnRunAlgorithmButtonClicked()
{
    setCursor(Qt::WaitCursor);
    ui->gridRunAlgorithmButton->setDisabled(true);
    QCoreApplication::processEvents();

    QTableWidgetItem *minSupCell = ui->gridParametersTable->item(0, 1);
    if(minSupCell) {
        QString cellText = minSupCell->text().trimmed().replace(",", ".");
        if(cellText == "") {
            QMessageBox::critical(this, "Error", "Minimum support not entered");

            setCursor(Qt::ArrowCursor);
            ui->gridRunAlgorithmButton->setDisabled(false);

            return;
        }

        bool success;
        float minSup = cellText.toFloat(&success);

        if(success) {
            if(minSup <= 0 || minSup > 1) {
                QMessageBox::critical(this, "Error", "Minimum support must be in scope (0, 1]");

                setCursor(Qt::ArrowCursor);
                ui->gridRunAlgorithmButton->setDisabled(false);

                return;
            }

            _gridTab->onRunAlgorithmButtonClicked(_gridScene, minSup);
        } else {
            QMessageBox::critical(this, "Error", "Minimum support must be number (float or integer)");
        }
    } else {
        QMessageBox::critical(this, "Error", "Minimum support not entered");
    }

    setCursor(Qt::ArrowCursor);
    ui->gridRunAlgorithmButton->setDisabled(false);
}


void MainWindow::frequentItemsetConfig()
{
    QString inputFilePath = _frequentItemsetTab->getInputFilePath();
    QString outputFilePath = _frequentItemsetTab->getOutputFilePath();
    ui->freqInputFileLine->setText(inputFilePath);
    ui->freqOutputFileLine->setText(outputFilePath);

    QTableWidgetItem *headerItem1 = new QTableWidgetItem(QString("Parameter"), QTableWidgetItem::Type);
    ui->freqParametersTable->setHorizontalHeaderItem(0, headerItem1);

    QTableWidgetItem *headerItem2 = new QTableWidgetItem(QString("Value"), QTableWidgetItem::Type);
    ui->freqParametersTable->setHorizontalHeaderItem(1, headerItem2);

    QTableWidgetItem *headerItem3 = new QTableWidgetItem(QString("Example"), QTableWidgetItem::Type);
    ui->freqParametersTable->setHorizontalHeaderItem(2, headerItem3);

    QTableWidgetItem *headerItem4 = new QTableWidgetItem(QString(""), QTableWidgetItem::Type);
    ui->freqParametersTable->setItem(0, 1, headerItem4);

    QTableWidgetItem *parameterCell = ui->freqParametersTable->item(0, 0);
    if(parameterCell) {
        parameterCell->setFlags(parameterCell->flags() & ~Qt::ItemIsEditable);
    } else {
        QMessageBox::critical(this, "Error", "Problem with getting parameter cell");
    }

    QTableWidgetItem *exampleCell = ui->freqParametersTable->item(0, 2);
    if(exampleCell) {
        exampleCell->setFlags(exampleCell->flags() & ~Qt::ItemIsEditable);
    } else {
        QMessageBox::critical(this, "Error", "Problem with getting example cell");
    }

    ui->freqParametersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    int rowCount = ui->freqParametersTable->rowCount();
    int columnCount = ui->freqParametersTable->columnCount();

    for(int row = 0; row < rowCount; row++) {
        for(int col = 0; col < columnCount; col++) {
            QTableWidgetItem *item = ui->freqParametersTable->item(row, col);
            if(item) {
                item->setTextAlignment(Qt::AlignCenter);
            }
        }
    }

    _frequentItemsetScene = new QGraphicsScene();
    ui->freqGraphicsView->setScene(_frequentItemsetScene);
}

double MainWindow::minkowskiDistance(const QVector<double> &vec1, const QVector<double> &vec2, double p)
{
    if (vec1.size() != vec2.size()) {
        throw std::invalid_argument("Vectors must have the same dimension.");
    }

    double sum = 0.0;
    for (int i = 0; i < vec1.size(); ++i) {
        sum += std::pow(std::abs(vec1[i] - vec2[i]), p);

    }
    return std::pow(sum, 1.0 / p);
}

double MainWindow::mahalanobisDistance(const QVector<double> &vec1, const QVector<double> &vec2)
{
    int n = vec1.size();
    // 1. Napravi 2xN matricu od vektora
    QVector<QVector<double>> data = {vec1, vec2};
    // 2. Izračunaj srednju vrednost po koloni
    QVector<double> mean(n, 0.0);
    for (int i = 0; i < n; ++i) {
        mean[i] = (vec1[i] + vec2[i]) / 2.0;
    }
    // 3. Izračunaj kovarijacionu matricu (n x n)
    QVector<QVector<double>> covMatrix(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < 2; ++k) {  // 2 vektora
                sum += (data[k][i] - mean[i]) * (data[k][j] - mean[j]);
            }
            covMatrix[i][j] = sum / 1.0;  // delimo sa (2 - 1) = 1
        }
    }// 4. Inverzija kovarijacione matrice (pretpostavimo dijagonalnu za jednostavnost)
    QVector<QVector<double>> invCovMatrix(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        if (covMatrix[i][i] == 0.0) {
            throw std::runtime_error("Covariance matrix is singular.");
        }
        invCovMatrix[i][i] = 1.0 / covMatrix[i][i];
    }// 5. diff = vec1 - vec2
    QVector<double> diff(n);
    for (int i = 0; i < n; ++i) {
        diff[i] = vec1[i] - vec2[i];
    }// 6. temp = invCovMatrix * diff
    QVector<double> temp(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            temp[i] += invCovMatrix[i][j] * diff[j];
        }
    }// 7. distance = sqrt(diff^T * temp)
    double distance = 0.0;
    for (int i = 0; i < n; ++i) {
        distance += diff[i] * temp[i];
    }
    return std::sqrt(distance);
}

double MainWindow::cosineDistance(const QVector<double> &vec1, const QVector<double> &vec2)
{
    if (vec1.size() != vec2.size()) {
        qWarning() << "Vektori nisu iste duzine!";
        return -1.0;
    }

    double dotProduct = 0.0;
    double normVec1 = 0.0;
    double normVec2 = 0.0;

    for (int i = 0; i < vec1.size(); ++i) {
        dotProduct += vec1[i] * vec2[i];
        normVec1 += vec1[i] * vec1[i];
        normVec2 += vec2[i] * vec2[i];
    }

    normVec1 = std::sqrt(normVec1);
    normVec2 = std::sqrt(normVec2);

    if (qFuzzyIsNull(normVec1) || qFuzzyIsNull(normVec2)) {
        qWarning() << "Jedan od vektora ima nultu normu!";
        return 0.0;  // Sličnost je 0 kad je bar jedan vektor nula
    }

    double similarity = dotProduct / (normVec1 * normVec2);
    // Opcionalno: clamp za stabilnost
    similarity = std::clamp(similarity, -1.0, 1.0);

    return similarity;
}

int MainWindow::hammingDistance(const QVector<double> &vec1, const QVector<double> &vec2)
{
    if (vec1.size() != vec2.size()) {
        qWarning() << "Vektori nisu iste duzine!";
        return -1;
    }

    int distance = 0;
    for (int i = 0; i < vec1.size(); ++i) {
        // Pretpostavljamo da su vrednosti 0 ili 1, pa samo proveravamo razliku
        if (vec1[i] != vec2[i]) {
            distance++;  // Povećaj rastojanje ako su vrednosti različite
        }
    }

    return distance;
}

double MainWindow::jaccardCoefficient(const QVector<double> &vec1, const QVector<double> &vec2)
{
    if (vec1.size() != vec2.size()) {
        qWarning() << "Vektori nisu iste dužine!";
            return -1.0;  // Greška
    }

    int m11 = 0; // Broj pozicija gde su oba 1
    int m10 = 0; // Broj pozicija gde je vec1=1, vec2=0
    int m01 = 0; // Broj pozicija gde je vec1=0, vec2=1

    for (int i = 0; i < vec1.size(); ++i) {
        if (vec1[i] == 1.0 && vec2[i] == 1.0) {
            m11++;
        } else if (vec1[i] == 1.0 && vec2[i] == 0.0) {
            m10++;
        } else if (vec1[i] == 0.0 && vec2[i] == 1.0) {
            m01++;
        }

    }

    int denominator = m11 + m10 + m01;
    if (denominator == 0) {
        return 0.0;  // Da se izbegne deljenje nulom
    }
    qDebug()<<m11<<m10<<m01<<denominator<< static_cast<double>(m11)<<static_cast<double>(m11) / denominator<<1 - static_cast<double>(m11) / denominator;
    return  static_cast<double>(m11) / denominator;
}


QVector<double> MainWindow::parseVector(const QString &filePath)
{
    QVector<double> vector;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->leRes->setText("ERROR: The file could not be opened. Please make sure you have selected the correct file with corrected form or entered a valid path.");
        qWarning() << "Error opening file:" << filePath;
        return vector;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList numbers = line.split(",");
        for (const QString& num : numbers) {
            bool ok;
            double value = num.toDouble(&ok);
            qDebug()<<value;
            if (ok) {
                vector.append(value);
            } else {
                ui->leRes->setText("ERROR: The file could not be opened. Please make sure you have selected the correct file with corrected form or entered a valid path.");
                qWarning() << "Failed to parse number:" << num;

            }
        }
    }

    file.close();
    return vector;
}
#include <QtAlgorithms>
uint qHash(const QSet<int> &key, uint seed = 0) {
    uint result = seed;
    for (int val : key)
        result ^= qHash(val, seed);
    return result;
}
bool containsAll(const QSet<int> &transaction, const QSet<int> &itemset) {
    for (int x : itemset)
        if (!transaction.contains(x))
            return false;
    return true;
}

bool containsSet(const QVector<QSet<int>> &list, const QSet<int> &set) {
    for (const QSet<int> &s : list) {
        if (s == set)
            return true;
    }
    return false;
}

// Provera da li su svi (k-1)-podskupovi kandidata retki
bool allSubsetsRare(const QSet<int> &candidate, const QVector<QSet<int>> &prevRare) {
    for (int item : candidate) {
        QSet<int> subset = candidate;
        subset.remove(item);
        if (!containsSet(prevRare, subset))
            return false;
    }
    return true;
}

void MainWindow::findRareItemsets(const QString &filename) {
    double MIN_SUPPORT = ui->leMinSupp->text().toDouble();
    QString output1 = ui->lePbOutputRare->text();

    // --- 1. Učitavanje transakcija
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Ne mogu da otvorim fajl!";
        return;
    }
    QTextStream in(&file);

    QVector<QVector<int>> database;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith('%') || line.startsWith('@'))
            continue;
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        QVector<int> transaction;
        for (auto &p : parts) {
            transaction.append(p.toInt());
        }
        std::sort(transaction.begin(), transaction.end());
        database.append(transaction);
    }
    file.close();

    int databaseSize = database.size();
    int minsupAbsolute = int(std::ceil(MIN_SUPPORT * databaseSize));

    // --- 2. Prvi scan: brojanje pojedinačnih stavki
    QMap<int,int> mapItemCount;
    for (auto &t : database) {
        for (int item : t) {
            mapItemCount[item]++;
        }
    }

    // --- 3. Inicijalizacija: razdvajamo česte vs. retke vel.1
    QVector<int> frequent1;
    QMap<QVector<int>,int> supportCount;
    for (auto it = mapItemCount.constBegin(); it != mapItemCount.constEnd(); ++it) {
        if (it.value() >= minsupAbsolute) {
            frequent1.append(it.key());
        } else {
            supportCount[{ it.key() }] = it.value();
        }
    }
    std::sort(frequent1.begin(), frequent1.end());

    // --- 4. Kandidati veličine 2
    using Itemset = QVector<int>;
    using Cand = QPair<Itemset,int>;
    QVector<Cand> candidates;
    for (int i = 0; i < frequent1.size(); ++i) {
        for (int j = i+1; j < frequent1.size(); ++j) {
            candidates.append({ { frequent1[i], frequent1[j] }, 0 });
        }
    }

    // Helper: proverava da li su svi (k-1)-podskupovi kandidata prisutni u nivou levelK_1
    auto allSubsetsFrequent = [&](const Itemset &cand, const QSet<Itemset> &setK_1) {
        for (int removePos = 0; removePos < cand.size(); ++removePos) {
            Itemset subset = cand;
            subset.removeAt(removePos);
            if (!setK_1.contains(subset)) return false;
        }
        return true;
    };

    // --- 5. Glavna petlja za k=2,3,...
    QVector<Itemset> levelK_1;
    int k = 2;
    while (!candidates.isEmpty()) {
        // reset podrške
        for (auto &c : candidates) c.second = 0;

        // scan DB i broj podrške
        for (auto &t : database) {
            for (auto &c : candidates) {
                const Itemset &ci = c.first;
                int i = 0, j = 0;
                while (i < t.size() && j < ci.size()) {
                    if (t[i] == ci[j]) { ++i; ++j; }
                    else if (t[i] < ci[j]) ++i;
                    else break;
                }
                if (j == ci.size()) ++c.second;
            }
        }

        // separacija
        QVector<Itemset> levelK;
        QSet<Itemset> setPrev(levelK_1.begin(), levelK_1.end());
        for (auto &c : candidates) {
            if (c.second >= minsupAbsolute) levelK.append(c.first);
            else supportCount[c.first] = c.second;
        }

        std::sort(levelK.begin(), levelK.end());
        QSet<Itemset> setLevelK(levelK.begin(), levelK.end());

        // generisanje sledećih kandidata
        QVector<Cand> next;
        for (int i = 0; i < levelK.size(); ++i) {
            for (int j = i+1; j < levelK.size(); ++j) {
                const Itemset &a = levelK[i];
                const Itemset &b = levelK[j];
                bool prefixEq = true;
                for (int x = 0; x < k-1; ++x) {
                    if (a[x] != b[x]) { prefixEq = false; break; }
                }
                if (!prefixEq) break;
                Itemset comb = a;
                comb.append(b.last());
                if (allSubsetsFrequent(comb, setLevelK)) next.append({ comb, 0 });
            }
        }

        levelK_1 = std::move(levelK);
        candidates = std::move(next);
        ++k;
    }

    // --- 6. Filtriraj minimalističke retke skupove
    QMap<Itemset,int> minimalRare;
    for (auto it = supportCount.constBegin(); it != supportCount.constEnd(); ++it) {
        const Itemset &iset = it.key();
        bool minimal = true;
        for (int pos = 0; pos < iset.size(); ++pos) {
            Itemset sub = iset;
            sub.removeAt(pos);
            if (supportCount.contains(sub)) { minimal = false; break; }
        }
        if (minimal) minimalRare[iset] = it.value();
    }

    // --- 7. Ispis
    qDebug() << "Minimalni retki skupovi (<" << minsupAbsolute << "):";
    for (auto it = minimalRare.constBegin(); it != minimalRare.constEnd(); ++it) {
        qDebug() << it.key() << "#SUP:" << it.value();
    }

    QFile outF(output1);
    if (!outF.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Ne mogu da otvorim izlazni fajl:" << output1;
    } else {
        QTextStream out(&outF);
        for (auto it = minimalRare.constBegin(); it != minimalRare.constEnd(); ++it) {
            QString s;
            for (int v : it.key()) s += QString::number(v) + " ";
            out << s.trimmed() << " #SUP: " << it.value() << "\n";
        }
        outF.close();
    }

    QString preview;
    for (auto it = minimalRare.constBegin(); it != minimalRare.constEnd(); ++it) {
        QString s;
        for (int v : it.key()) s += QString::number(v) + " ";
        preview += "{" + s.trimmed() + "} -> " + QString::number(it.value()) + "\n";
    }
    ui->textEdit->setText(preview);
}



void MainWindow::freqOnBrowseButtonClicked()
{
    QString filePath = _frequentItemsetTab->onBrowseButtonClicked();
    ui->freqInputFileLine->setText(filePath);
}


void MainWindow::freqOnChangeButtonClicked()
{
    QString filePath = _frequentItemsetTab->onChangeButtonClicked();
    ui->freqOutputFileLine->setText(filePath);
}


void MainWindow::freqOnRunAlgorithmButtonClicked()
{
    setCursor(Qt::WaitCursor);
    ui->freqRunAlgorithmButton->setDisabled(true);
    QCoreApplication::processEvents();

    QTableWidgetItem *minSupCell = ui->freqParametersTable->item(0, 1);
    if(minSupCell) {
        QString cellText = minSupCell->text().trimmed().replace(",", ".");
        if(cellText == "") {
            QMessageBox::critical(this, "Error", "Minimum support not entered");

            setCursor(Qt::ArrowCursor);
            ui->freqRunAlgorithmButton->setDisabled(false);

            return;
        }

        bool success;
        float minSup = cellText.toFloat(&success);

        if(success) {
            if(minSup <= 0 || minSup > 1) {
                QMessageBox::critical(this, "Error", "Minimum support must be in scope (0, 1]");

                setCursor(Qt::ArrowCursor);
                ui->freqRunAlgorithmButton->setDisabled(false);

                return;
            }

            _frequentItemsetTab->onRunAlgorithmButtonClicked(_frequentItemsetScene, minSup);
        } else {
            QMessageBox::critical(this, "Error", "Minimum support must be number (float or integer)");
        }
    } else {
        QMessageBox::critical(this, "Error", "Minimum support not entered");
    }

    setCursor(Qt::ArrowCursor);
    ui->freqRunAlgorithmButton->setDisabled(false);
}


void MainWindow::freqOnForwardButtonClicked()
{
    setCursor(Qt::WaitCursor);
    _frequentItemsetTab->onForwardButtonClicked(_frequentItemsetScene);
    setCursor(Qt::ArrowCursor);
}

void MainWindow::changeParams()
{
    if (ui->comboBox->currentText() == "Minkowski Distance"){
        ui->leParam->setReadOnly(false);
        ui->leParam->setText("");
    }
    else{
        ui->leParam->setText("There is no parameter defined for this distance.");
        ui->leParam->setReadOnly(true);

    }
}

void MainWindow::pbChooseVector1()
{
    QString filePath = _frequentItemsetTab->onBrowseButtonClicked();
    ui->leVec1->setText(filePath);
}

void MainWindow::pbChooseVector2()
{
    QString filePath = _frequentItemsetTab->onBrowseButtonClicked();
    ui->leVec2->setText(filePath);
}

void MainWindow::pbCompute()
{
    QString dist = ui->comboBox->currentText();
    QString path1 = ui->leVec1->text();
    QString path2 = ui->leVec2->text();
    QVector<double> vec1 = MainWindow::parseVector(path1);
    QVector<double> vec2 = MainWindow::parseVector(path2);
    if (ui->leRes->text() == "ERROR: The file could not be opened. Please make sure you have selected the correct file with corrected form or entered a valid path."){
        return;
    }
    if (vec1.size() != vec2.size()){
        ui->leRes->setText("ERROR: Vectors are not of the same dimensions.");
        return;
        }

    ui->label->setText(QString::number(vec1[0]));
    double p = ui->leParam->text().toDouble();
    if (dist == "Minkowski Distance"){

        double res = MainWindow::minkowskiDistance(vec1, vec2, p);
        ui->leRes->setText(QString::number(res));
    }


    if (dist == "Mahalanobis Distance"){
        int dim = vec1.size();

        // Kreiranje identitetske matrice dim x dim
        QVector<QVector<double>> identityMatrix(dim, QVector<double>(dim, 0.0));
        for (int i = 0; i < dim; ++i) {
            identityMatrix[i][i] = 1.0;
        }
        double res = MainWindow::mahalanobisDistance(vec1, vec2);
        ui->leRes->setText(QString::number(res));
    }
    if (dist == "Cosine Distance"){
        double res = MainWindow::cosineDistance(vec1, vec2);
        ui->leRes->setText(QString::number(res));
    }
    if (dist == "Hamming Distance"){
        for (int i = 0; i < vec1.size(); ++i) {
        if (!(vec1[i] == 0 or vec1[i] == 1)){
            ui->leRes->setText("Vectors value must be 1 or 0");
                return;
        }
        if (!(vec2[i] == 0 or vec2[i] == 1)){
            ui->leRes->setText("Vectors value must be 1 or 0");
                return;
        }
        }
        int res = MainWindow::hammingDistance(vec1, vec2);
        ui->leRes->setText(QString::number(res));

    }
    if (dist == "Jaccard coefficient"){
        for (int i = 0; i < vec1.size(); ++i) {
        if (!(vec1[i] == 0 or vec1[i] == 1)){
                ui->leRes->setText("Vectors value must be 1 or 0");
                return;
        }
        if (!(vec2[i] == 0 or vec2[i] == 1)){
                ui->leRes->setText("Vectors value must be 1 or 0");
                return;
        }
        }
    double res = MainWindow::jaccardCoefficient(vec1, vec2);
    ui->leRes->setText(QString::number(res));

    }
}

void MainWindow::pbChooseApr()
{
    QString filePath = _frequentItemsetTab->onBrowseButtonClicked();
    ui->leApr->setText(filePath);
}

void MainWindow::pbFindRare()
{
    MainWindow::findRareItemsets(ui->leApr->text());
}

void MainWindow::pbChooseOutput()
{
    QString filePath = _frequentItemsetTab->onBrowseButtonClicked();
    ui->lePbOutputRare->setText(filePath);
}


MainWindow::~MainWindow()
{
    delete ui;
    delete _gridTab;
    delete _gridScene;
    delete _frequentItemsetTab;
    delete _frequentItemsetScene;
}
