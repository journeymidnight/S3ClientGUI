#ifndef QS3CONFIG_H
#define QS3CONFIG_H

#include <QObject>

class QS3Config : public QObject
{
    Q_OBJECT

public:
    enum ConfigParseResult
    {
        ConfigOK,
        ConfigNotExist,
        ConfigParaseError,
    };

    struct S3Config
    {
        QString endpoint;
        QString accessKey;
        QString secretKey;
        QString schema;
    };

public:
    static QS3Config *Instance();
	inline S3Config &getS3Config() { return m_s3config; }
    ConfigParseResult loadConfigFile();
    void saveConfigFile();
	bool s3ConfigIsValid();

protected:
    explicit QS3Config(QObject *parent = nullptr);

private:
    const QString DEFAULT_CONFIG = "./.S3Config.in";
    static QS3Config *_instance;
	S3Config m_s3config;
};

#endif // QS3CONFIG_H
