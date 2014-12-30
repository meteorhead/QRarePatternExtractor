#ifndef RAREPATTERN_H
#define RAREPATTERN_H

#include <QWidget>
#include <QString>
#include <QFileDialog>
#include <QList>
#include <QHash>
#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
class QPushButton;
class QTableWidget;



namespace Ui {
class RarePattern;
}

class RarePattern : public QWidget
{
    Q_OBJECT
    
public:
    explicit RarePattern(QWidget *parent = 0);
    void buildItemSetDB(const QString &line); //Build a set of 1-itemsets in memory with all unique item sets
    QList<QString> & genCandidates(const QHash<QString,int> &DBItems);


    ~RarePattern();
signals:
    void signalGenerateFirstItemsets();
    void signalGenerateSecondItemSets();
    void signalFilterCandidates();
    void signalGenerateK_1itemsets();
public slots:
    void openFileDialog();
    void processSelectedFile(const QString &);
    void generate1_ItemsetCandidates();
    void genCandidates2();
    void genK_1Candidates();
    void filterCandidates();  // delete candidates that don't appear on the file or > min_supp
    void openRarePatternDialog();
    void saveToFile();

private:
    Ui::RarePattern *ui;
    QString chosenFile;
    int minSupp; // minimum support
    int longestTransaction; // holds the size of data to browse
    QHash<QString, int> rarePatternsHash; // QHash to hold filtered k-itemsets and their number of occurences
    QHash<QChar, int> items; // holds single item databases with their respective # of occurences
    QString database; // database of all items in the file
    QFileDialog fileDialog,saveLocation;
    int depthK;
    QStringList candidatesList1,candidatesList2,k_1Candidates;
    QDialog rareDialog;
    QTableWidget *tablewidget;
    QPushButton *savePatternsToFile;
};

#endif // RAREPATTERN_H
