#include "../utils/util.h"
#include "../utils/os.h"
#include "../utils/tcp.h"
#include "../utils/video.h"
#include "../utils/threading.h"

#include <QDebug>
#include <QString>
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
#include <QJsonArray>
#include <QList>
#include <unistd.h> // sleep usleep


int main()
{
    QString test = "0000000002";
   
    QStringRef subString(&test,  test.length() - 4, 4); // subString contains "is"
     qDebug() << subString;
}
