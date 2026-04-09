#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStringList>

QTcpServer *server;

void handleClient(QTcpSocket *socket) {
    QObject::connect(socket, &QTcpSocket::readyRead, [socket]() {
        QByteArray data = socket->readAll();
        QString request = QString::fromUtf8(data).trimmed();

        qDebug() << "Request:" << request;

        QStringList parts = request.split("&");
        QString response = "error";

        if (parts.size() > 0) {
            QString cmd = parts[0];

            if (cmd == "reg" && parts.size() == 4) {
                QString login = parts[1];
                QString password = parts[2];
                QString email = parts[3];

                QSqlQuery query;
                query.prepare("INSERT INTO users (login, password, email) VALUES (?, ?, ?)");
                query.addBindValue(login);
                query.addBindValue(password);
                query.addBindValue(email);

                if (query.exec()) {
                    response = "reg+&" + login;
                } else {
                    qDebug() << "REG error:" << query.lastError().text();
                    response = "reg-";
                }
            }
            else if (cmd == "auth" && parts.size() == 3) {
                QString login = parts[1];
                QString password = parts[2];

                QSqlQuery query;
                query.prepare("SELECT id FROM users WHERE login = ? AND password = ?");
                query.addBindValue(login);
                query.addBindValue(password);

                if (query.exec() && query.next()) {
                    response = "auth+&" + login;
                } else {
                    response = "auth-";
                }
            }
            else if (cmd == "stat" && parts.size() == 2) {
                QString login = parts[1];

                QSqlQuery query;
                query.prepare("SELECT solved, wrong, score FROM users WHERE login = ?");
                query.addBindValue(login);

                if (query.exec() && query.next()) {
                    response = "stat&" +
                               query.value(0).toString() + "&" +
                               query.value(1).toString() + "&" +
                               query.value(2).toString();
                } else {
                    response = "stat-";
                }
            }
        }

        qDebug() << "Response:" << response;

        socket->write(response.toUtf8());
        socket->waitForBytesWritten(1000);
    });
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("users.db");

    if (!db.open()) {
        qDebug() << "DB error:" << db.lastError().text();
        return -1;
    }

    qDebug() << "DB connected";

    QSqlQuery query;
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "login TEXT UNIQUE, "
            "password TEXT, "
            "email TEXT UNIQUE, "
            "solved INTEGER DEFAULT 0, "
            "wrong INTEGER DEFAULT 0, "
            "score INTEGER DEFAULT 0)"
            )) {
        qDebug() << "Create table error:" << query.lastError().text();
        return -1;
    }

    qDebug() << "Table users ready";

    server = new QTcpServer();

    QObject::connect(server, &QTcpServer::newConnection, []() {
        QTcpSocket *client = server->nextPendingConnection();
        handleClient(client);
    });

    server->listen(QHostAddress::Any, 33333);
    qDebug() << "Server started on port 33333";

    return a.exec();
}
