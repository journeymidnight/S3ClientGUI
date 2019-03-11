#include "qs3config.h"
#include <QSettings>

QS3Config::QS3Config(QObject *parent) : QObject(parent)
{

}

QS3Config *QS3Config::_instance = nullptr;
QS3Config *QS3Config::Instance()
{
	if (nullptr == _instance)
		_instance = new QS3Config();
	
    return _instance;
}

QS3Config::ConfigParseResult QS3Config::loadConfigFile()
{
    QSettings settings("oss", "S3ClientGUI");
	m_s3config.endpoint = settings.value("endpoint", "oss-cn-north-1.unicloudsrv.com").toString();
	m_s3config.schema = settings.value("schema", "").toString();
	m_s3config.accessKey = settings.value("accessKey", "").toString();
	m_s3config.secretKey = settings.value("secretKey", "").toString();

    return  ConfigParseResult::ConfigOK;
}

void QS3Config::saveConfigFile()
{
	QSettings settings("oss", "S3ClientGUI");
    settings.setValue("endpoint", m_s3config.endpoint);
    settings.setValue("schema", m_s3config.schema);
    settings.setValue("accessKey", m_s3config.accessKey);
    settings.setValue("secretKey", m_s3config.secretKey);

    return;
}

bool QS3Config::s3ConfigIsValid()
{
	if (m_s3config.endpoint.isEmpty() ||
		m_s3config.schema.isEmpty() ||
		m_s3config.accessKey.isEmpty() ||
		m_s3config.secretKey.isEmpty())
		return false;

	return true;
}