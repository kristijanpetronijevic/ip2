#ifndef FREQUENTITEMSET_H
#define FREQUENTITEMSET_H

#include <QString>
#include <QCoreApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QMap>
#include <QTextEdit>
#include <QDesktopServices>


class FrequentItemset
{
public:
    // Constructors
    FrequentItemset();

    // Destructors
    ~FrequentItemset();

    // Getters
    QString getInputFilePath();
    QString getOutputFilePath();

    // Slots
    QString onBrowseButtonClicked();
    QString onChangeButtonClicked();
    void onRunAlgorithmButtonClicked(QGraphicsScene *scene, const double minSupport);
    void onForwardButtonClicked(QGraphicsScene *scene);

private:
    // Fields
    QString _inputFilePath;
    QString _outputFilePath;
    QString _inputOpenFilePath;
    QString _outputOpenFilePath;
    int _nodeRadius;
    QTextEdit *_editor;
    QMap<QVector<int>, int> _nodesSupport;
    QVector<QVector<int>> _transactions;
    QMap<int, int> _itemsFrequencies;
    QMap<QVector<int>, int> _setsFrequencies;
    QVector<QVector<int>> _sortedTransactions;
    QMap<QVector<int>, QVector<QVector<int>>> _childrenMap;
    QMap<QVector<int>, QVector<QVector<int>>> _currentChildrenMap;
    QMap<QVector<int>, QPointF> _nodePositions;
    QVector<QGraphicsEllipseItem*> _pendingRemovalEllipses;
    bool _removalColoring;
    double _minSupport;
    QMap<QVector<int>, int> _frequentItemsets;

    // Functions
    bool readFile();
    void findItemFrequencies();
    void sortTransactions();
    void findSetsFrequencies();
    void removeRareItemsets();
    void findChildren();
    bool childExists(const QVector<QVector<int>> &childrenList, const QVector<int> &child);
    void drawTree(QGraphicsScene *scene);
    void saveFile(const QVector<QVector<int>> &frequentItemsets);
};


#endif // FREQUENTITEMSET_H
