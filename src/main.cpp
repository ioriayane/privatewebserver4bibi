#include <QtCore/QCoreApplication>
#include <QtHttpServer>
#include <QtHttpServer/QHttpServerResponder>
#include <QCommandLineParser>
#include <QFile>
#include <QUrl>

class WebServer : public QAbstractHttpServer
{
    QString m_bookShelf;
    QMimeDatabase m_MimeDb;

public:
    WebServer(QObject *parent = nullptr) : QAbstractHttpServer(parent)
    {
        m_bookShelf = QCoreApplication::applicationDirPath();
    }

    bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) override
    {
        if (request.url().path() == "/") {
            makeResponder(request, socket)
                    .write(QCoreApplication::applicationName().toUtf8(), "text/plain",
                           QHttpServerResponder::StatusCode::Ok);
        } else {
            QFileInfo file_info(request.url().path());
            QString path;
            bool epub_candidate = false;
            if (request.url().path().indexOf("/bibi-bookshelf/") == 0) {
                path = m_bookShelf + request.url().path().remove("/bibi-bookshelf");
                epub_candidate = (file_info.suffix() == "epub");
            } else {
                path = ":" + request.url().path();
            }
            qDebug() << path << file_info.suffix() << m_MimeDb.mimeTypeForFile(file_info).name();

            if (!QFile::exists(path)) {
                makeResponder(request, socket).write(QHttpServerResponder::StatusCode::NotFound);
            } else if (epub_candidate && !isEbook(path)) {
                makeResponder(request, socket)
                        .write(QHttpServerResponder::StatusCode::InternalServerError);
            } else {
                QFile *file = new QFile(path);
                if (file->open(QFile::ReadOnly)) {
                    makeResponder(request, socket)
                            .write(file, m_MimeDb.mimeTypeForFile(file_info).name().toUtf8(),
                                   QHttpServerResponder::StatusCode::Ok);
                } else {
                    makeResponder(request, socket)
                            .write(QHttpServerResponder::StatusCode::InternalServerError);
                }
            }
        }
        return true;
    }

    void setBookShelf(const QString &bookShelf) { m_bookShelf = bookShelf; }

private:
    bool isEbook(const QString &path)
    {
        bool ret = false;
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            QByteArray data = file.read(105);
            ret = (data.mid(30, 28) == QByteArray("mimetypeapplication/epub+zip"));
            file.close();
        }
        return ret;
    }
};

// command
// 0 : run check
//      true : already running
//      false : stopped
// 1 : kill if i am running
//      true : killed
//      false : do nothing
bool processControl(int command)
{
    QProcess process;
    int pid = 0;
    QFileInfo app_info(QCoreApplication::applicationFilePath());
    QStringList args;

#ifdef Q_OS_WIN
    args << QStringLiteral("/OUTPUT:STDOUT");
    args << QStringLiteral("PROCESS");
    args << QStringLiteral("GET");
    args << QStringLiteral("Name,ProcessId");
    process.start("wmic.exe", args);
    process.waitForFinished(10000);
#else
    args << QStringLiteral("-eo");
    args << QStringLiteral("pid,command");
    process.start("ps", args);
    process.waitForFinished(10000);
#endif

    QStringList lines =
            QString::fromLocal8Bit(process.readAllStandardOutput()).split(QStringLiteral("\n"));
    foreach (const QString &line, lines) {
        QStringList items = line.split(QStringLiteral(" "), Qt::SkipEmptyParts);
        if (items.length() >= 2) {
#ifdef Q_OS_WIN
            QFileInfo p_info(items.at(0));
            QString pid_str = items.at(1);
#else
            QFileInfo p_info(items.at(1));
            QString pid_str = items.at(0);
#endif
            if (p_info.fileName() == app_info.fileName()
                && pid_str.toInt() != QCoreApplication::applicationPid()) {

                pid = pid_str.toInt();

                if (command == 1) {
                    // kill
                    args.clear();
#ifdef Q_OS_WIN
                    args.append(QStringLiteral("/pid"));
                    args.append(pid_str);
                    args.append(QStringLiteral("/F"));
                    process.start(QStringLiteral("taskkill"), args);
                    process.waitForFinished(10000);
#else
                    args << pid_str;
                    process.start(QStringLiteral("kill"), args);
                    process.waitForFinished(10000);
#endif
                    qInfo().noquote().nospace() << "Stop server(" << pid_str << ").";
                }
            }
        }
    }

    return (pid > 0);
}

bool runTest(const quint16 port)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QString("http://localhost:%1/").arg(port)));
    request.setAttribute(QNetworkRequest::SynchronousRequestAttribute, true);
    QNetworkReply *reply = manager.get(request);
    reply->deleteLater();
    return (QCoreApplication::applicationName().toUtf8() == reply->readAll());
}

void notifyUrl(const quint16 port)
{
    qInfo().noquote().nospace()
            << QStringLiteral("Url example:http://localhost:%1/bibi/index.html?book=example.epub")
                       .arg(port);
}

int main(int argc, char *argv[])
{
    // bibisprivatewebserver --book-shelf ./bibi-bookshelf --port 8080

    QCoreApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("Private Web Server for Bibi"));
    app.setApplicationVersion(QStringLiteral("0.1"));

    // Parse command line options
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption p_port_no(QStringLiteral("port"), QCoreApplication::tr("Port No."),
                                 QStringLiteral("no"));
    parser.addOption(p_port_no);
    QCommandLineOption p_book_folder(QStringLiteral("book-shelf"),
                                     QCoreApplication::tr("Ebook shelf folder path."),
                                     QStringLiteral("book-shelf"));
    parser.addOption(p_book_folder);
    QCommandLineOption p_stop(QStringLiteral("stop"), QCoreApplication::tr("Stop me."));
    parser.addOption(p_stop);
    parser.process(app.arguments());

    quint16 port_no = 52224;
    QString book_folder;
    if (parser.isSet(p_port_no)) {
        port_no = parser.value(p_port_no).toUInt();
    }
    if (parser.isSet(p_book_folder)) {
        book_folder = parser.value(p_book_folder);
    } else {
        book_folder = QCoreApplication::applicationDirPath();
    }

    qInfo().noquote().nospace() << QCoreApplication::applicationName()
                                << "! Ver:" << QCoreApplication::applicationVersion();

    if (parser.isSet(p_stop)) {
        // kill mode
        if (!processControl(1)) {
            qInfo().noquote().nospace() << "Nothing running server.";
        }
        return 0;
    } else {
        // normal mode

        // Run check
        bool running = false;
        if (processControl(0)) {
            if (runTest(port_no)) {
                running = true;
            }
        }

        // Start server
        if (running) {
            qInfo().noquote().nospace() << "Already running.";
            notifyUrl(port_no);
            return 0;
        } else {
            WebServer server;
            server.setBookShelf(book_folder);
            quint16 listen_port = server.listen(QHostAddress::LocalHost, port_no);
            if (listen_port == 0) {
                // listen failed
                qInfo().noquote().nospace() << "Fail(" << port_no << ").";
                return 1;
            } else {
                qInfo().noquote().nospace() << "Start server(" << listen_port << ")...";
                notifyUrl(listen_port);
                return app.exec();
            }
        }
    }
}
