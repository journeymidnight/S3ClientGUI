#include "editaccountdialog.h"
#include "ui_editaccountdialog.h"
#include "qs3config.h"

#include <QRegExp>
#include <QRegExpValidator>

EditAccountDialog::EditAccountDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditAccountDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setLayout(ui->verticalLayout);

    QRegExp urlRegExp("[0-9a-zA-Z]([-.\w]*[0-9a-zA-Z])*(:(0-9)*)*(\\/?)([a-zA-Z0-9\-\\.\\?\\,\\'\\/\\\\\\+&amp;%\\$#_]*)?");
    ui->lineEdit_Endpoint->setValidator(new QRegExpValidator(urlRegExp, this));
    QRegExp akRegExp("(^|[^A-Za-z0-9])[A-Za-z0-9]{20}(?![A-Za-z0-9])");
    ui->lineEdit_AccessKey->setValidator(new QRegExpValidator(akRegExp, this));
    QRegExp skRegExp("(^|[^A-Za-z0-9/+=])[A-Za-z0-9/+=]{40}(?![A-Za-z0-9/+=])");
    ui->lineEdit_SecretKey->setValidator(new QRegExpValidator(skRegExp, this));

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [ = ]() {
        QS3Config::S3Config &s3config = QS3Config::Instance()->getS3Config();
        s3config.endpoint = ui->lineEdit_Endpoint->text();
        s3config.accessKey = ui->lineEdit_AccessKey->text();
        s3config.secretKey = ui->lineEdit_SecretKey->text();
        if (ui->checkBox_https->isChecked())
            s3config.schema = "https";
        else
            s3config.schema = "http";

        QS3Config::Instance()->saveConfigFile();

        this->accept();
    });

    QS3Config::Instance()->loadConfigFile();
    QS3Config::S3Config s3config = QS3Config::Instance()->getS3Config();
    ui->lineEdit_Endpoint->setText(s3config.endpoint);
    ui->lineEdit_AccessKey->setText(s3config.accessKey);
    ui->lineEdit_SecretKey->setText(s3config.secretKey);
    if (0 == QString::compare("http", s3config.schema))
        ui->checkBox_https->setChecked(false);
    else if (0 == QString::compare("https", s3config.schema))
        ui->checkBox_https->setChecked(true);
}

EditAccountDialog::~EditAccountDialog()
{
    delete ui;
}
