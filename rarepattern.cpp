#include "rarepattern.h"
#include "ui_rarepattern.h"
#include <QDialog>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QListWidgetItem>
#include <math.h>

RarePattern::RarePattern(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RarePattern)
{
    ui->setupUi(this);
    ui->listWidget->setVisible(false);
    // This will hold the number of transactions per rare pattern
    longestTransaction = 0;
    depthK = 0;
    minSupp = ui->spinBox->text().toInt();
    if (minSupp <= 0) //if the spinbox doesn't contain a value, we assign it 3
        minSupp = 3;
    connect(ui->bConfirm,SIGNAL(clicked()),this,SLOT(openFileDialog()));
    connect(&fileDialog,SIGNAL(fileSelected(QString)),this,SLOT(processSelectedFile(QString)));
    connect(this,SIGNAL(signalGenerateFirstItemsets()),this,SLOT(generate1_ItemsetCandidates()));
    connect(this,SIGNAL(signalGenerateSecondItemSets()),this,SLOT(genCandidates2()));
    connect(this,SIGNAL(signalFilterCandidates()),this,SLOT(filterCandidates()));
    connect(this,SIGNAL(signalGenerateK_1itemsets()),this,SLOT(genK_1Candidates()));
    connect(ui->bExecute,SIGNAL(clicked()),this,SLOT(openRarePatternDialog()));

}

/* Method that opens a dialog to let user choose file containing patterns */
void RarePattern::openFileDialog()
{
    // apply filter to only show .txt files
    fileDialog.setNameFilter("(*.txt)");
    fileDialog.exec();
}


/* Method to process file to build item sets*/
void RarePattern::processSelectedFile(const QString & selected)
{
    //first we make sure that the string argument is not empty
    if (selected.isEmpty())
        return;
   chosenFile = selected;
   QFile file(chosenFile);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
           return;
   QTextStream in(&file);

   QString line = " ";
   int lengthofLine = 0;
   // browse the file line by line and append them the list widget
   while (!line.isNull()) {
           line.simplified();
           line.replace(" ","");
           buildItemSetDB(line);
           lengthofLine = line.length();
           if (lengthofLine > longestTransaction)
               longestTransaction = lengthofLine;
           line = in.readLine();
           if (!line.isEmpty())
                   new QListWidgetItem(line, ui->listWidget);

       }
   ui->listWidget->setVisible(true);
   emit signalGenerateFirstItemsets();

}

/* This method extract unique items from each transaction
 and add it to the memory database
 */
void RarePattern::buildItemSetDB(const QString & line)
{
   QHash<QChar,int>::iterator it;
   database.append(line);
        for (int i = 0; i < line.length(); i++)
        {  int numberOfOccurences = 0;

                    numberOfOccurences = line.count(line[i]);
                    if (numberOfOccurences > 1)
                       {
                        numberOfOccurences = 1;
                        items.remove(line[i]); //eliminate potential duplicates on the same line
                        }

                    items.insertMulti(line[i].toLatin1(),numberOfOccurences);

        }
}
/* Method to generate itemsets consisting of single items i.e "1", "2", "3" */
void RarePattern::generate1_ItemsetCandidates()
{

    QHash<QChar,int>::iterator it, lookForDuplicates;


    for (it = items.begin(); it != items.end(); it++)
    { int count = 0;
        for (lookForDuplicates = items.begin(); lookForDuplicates != items.end(); lookForDuplicates++)
        {
            if (it.key() == lookForDuplicates.key())
                count++;
         }
        if (items.count(it.key()) > 0 && items.count(it.key()) <= minSupp)
            rarePatternsHash.insert(it.key(),count);
    }

    emit signalGenerateSecondItemSets();
}

/* Uses candidates of size 1 (previous method) to Generate candidates of size 2: "12", "13"... */
void RarePattern::genCandidates2()

{

   depthK = 1;
   candidatesList1 = rarePatternsHash.keys();
   for (int i = 0; i < candidatesList1.size(); i++)
       for (int j = i + 1; j < candidatesList1.size(); j++)
           candidatesList2.append(candidatesList1.at(i)+" "+candidatesList1.at(j));




   emit signalFilterCandidates();


}
/* Filter candidates to keep relevant itemsets, eliminate duplicated itemsets, etc */
void RarePattern::filterCandidates()
{

    if (depthK < 2)
    {
    depthK++;
    for (int j = 0; j < candidatesList2.size(); j++ )
    {
        int count = 0;
            count = database.count(candidatesList2.at(j).simplified().replace(" ",""));

             if (count > 1 && count <= minSupp)
             {   QString str;
                 for (int i=0; i < candidatesList2.at(i).size(); i++)
                     str += candidatesList2.at(i)+" ";
                 rarePatternsHash.insert(candidatesList2.at(j),count);
                 k_1Candidates.append(candidatesList2.at(j));
             }

    }

    emit signalGenerateK_1itemsets();
    }

    else
    {

         QStringList candids;
          for (int j = 0; j < k_1Candidates.size(); j++ )
              candids.append(k_1Candidates.at(j).split(" "));


        candids.removeDuplicates();
        int count = 0;
        QString xp = candids.join("");
        QRegExp exp(xp);
        count = database.count(exp);




            if (count > 1 && count <= minSupp)

            {
                QHashIterator<QString, int> ii(rarePatternsHash);
                while (ii.hasNext()) {
                    ii.next();
                    QString s = ii.key();

                    s.simplified().replace(" ","");
                    QRegExp exp("["+xp+"]");
                        if(ii.key().count(exp) > 0)
                            rarePatternsHash.remove(ii.key());
                }

                rarePatternsHash.insert(xp,count);
            }




    }
    emit signalGenerateK_1itemsets();
  }



/* this method generates candidates of size greater then 2: "123" , "2456" etc */
void RarePattern::genK_1Candidates()
{
      depthK++;
    if (depthK <= longestTransaction)
    {
    QStringList newCandidates = k_1Candidates;
    k_1Candidates.clear();
    for (int i = 0; i < newCandidates.size(); i++)
            {

               QString c1 = newCandidates.at(i);
               QString c2 = newCandidates.at(i);
               QStringList lc1,lc2;
               lc1.append(c1);
               lc2.append(c2);
               QStringList candidates;
                 int k =0,h = 0;
               for (int i=0; i<c1.size();i++)
               {
               while ( h < depthK - 1)
               {

                   if (c1[i] == c2[i])
                       k++;
                   h++;
               }
               }

               if ( k == (depthK - 1))
                 {
                   QStringList sc1 = lc1.at(0).split(" ");
                   QStringList sc2 = lc2.at(0).split(" ");
                   candidates = sc1+sc2;
                   candidates.removeDuplicates();
                   QString can = candidates.join(" ");
                   k_1Candidates.append(can);

                 }

               }
              emit signalFilterCandidates();
    }

}
/* open a dialog with rare patterns */
void RarePattern::openRarePatternDialog()
{
    rareDialog.setWindowTitle("Motifs rares");
    QVBoxLayout *vL = new QVBoxLayout;
    tablewidget = new QTableWidget(rarePatternsHash.size(),2,&rareDialog);
    QStringList labels;
    labels << "Motifs rares" << " N° Transactions";
    tablewidget->setHorizontalHeaderLabels(labels);
    savePatternsToFile = new QPushButton("Sauvegarder");
    vL->addWidget(tablewidget);
    vL->addWidget(savePatternsToFile);
   connect(savePatternsToFile,SIGNAL(clicked()),this,SLOT(saveToFile()));
    rareDialog.setLayout(vL);
    QHash<QString, int>::Iterator it;
    int rows = 0;
    for ( it = rarePatternsHash.begin(); it != rarePatternsHash.end(); it++)
    {
        QTableWidgetItem *itemkey = new QTableWidgetItem (it.key());
                tablewidget->setItem(rows,0,itemkey);

        QTableWidgetItem *itemVal = new QTableWidgetItem (tr("%1").arg(it.value()));
                tablewidget->setItem(rows,1,itemVal);
        rows++;
    }
    rareDialog.show();

}
/* Save the rare patterns in the previous dialog to the build directory */
void RarePattern::saveToFile()
{


    QFile file("rarePatterns.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
    QTextStream out(&file);
    out << "Motifs rares " << "N Transactions\n";
    QHash<QString, int>::Iterator it;
    for ( it = rarePatternsHash.begin(); it != rarePatternsHash.end(); it++)
    {
        out << it.key() << "               " << it.value() << "\n";
    }



    file.close();
    rareDialog.close();
}

RarePattern::~RarePattern()
{
    delete ui;
}

