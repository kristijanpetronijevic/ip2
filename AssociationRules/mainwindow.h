#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QStringList>
#include <QCoreApplication>
#include <QTextStream>
#include <QHash>
#include <QStringList>
#include <QSet>
#include <QDebug>
#include "tabs/Grid/grid.h"
#include "tabs/FrequentItemset/frequentitemset.h"



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void gridOnBrowseButtonClicked();
    void gridOnChangeButtonClicked();
    void gridOnRunAlgorithmButtonClicked();

    void freqOnBrowseButtonClicked();
    void freqOnChangeButtonClicked();
    void freqOnRunAlgorithmButtonClicked();
    void freqOnForwardButtonClicked();

    void changeParams();
    void pbChooseVector1();
    void pbChooseVector2();
    void pbCompute();

    void pbChooseApr();
    void pbFindRare();
    void pbChooseOutput();


private:
    Ui::MainWindow *ui;
    Grid *_gridTab;
    QGraphicsScene *_gridScene;
    FrequentItemset *_frequentItemsetTab;
    QGraphicsScene *_frequentItemsetScene;

    void gridConfig();
    void frequentItemsetConfig();
    double minkowskiDistance(const QVector<double> &vec1, const QVector<double> &vec2, double p);
    double mahalanobisDistance(const QVector<double> &vec1, const QVector<double> &vec2);
    double cosineDistance(const QVector<double>& vec1, const QVector<double>& vec2);
    int hammingDistance(const QVector<double>& vec1, const QVector<double>& vec2);
    double jaccardCoefficient(const QVector<double> &vec1, const QVector<double> &vec2);

    QVector<double> parseVector(const QString& filePath);

    void findRareItemsets(const QString &filename);



};


#endif // MAINWINDOW_H
