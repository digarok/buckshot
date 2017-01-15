#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qfiledialog.h"
#include "qprocess.h"
#include "qdebug.h"
#include "qtimer.h"
#include "qmessagebox.h"
#include "qinputdialog.h"

const QString MainWindow::programName = QString("buckshot");
const QString MainWindow::version = QString("0.02");
const QString MainWindow::imageName = QString("saved");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Window Title
    this->setWindowTitle(QString("%1  v%2").arg(programName,version));

    // SET TEMP DIR FOR CACHE/BUILDING
    tmpDir = new QTemporaryDir();
    if (tmpDir->isValid()) {
        tmpDirPath = tmpDir->path();   // dir.path() returns the unique directory path
    } else {
        tmpDirPath = "/tmp/";          // fallback.  may not work on all OSes
    }

    // AND NOW THE PATHNAMES FOR OUR CACHE FILES
    inputImgPath = QString("%1/%2.bmp").arg(tmpDirPath).arg(imageName);
    previewImgPath = QString("%1/%2_Preview.bmp").arg(tmpDirPath).arg(imageName);

    // POPULATE FORMAT COMBOBOX
    QStringList outputFormats;
    outputFormats << "LR" << "DLR" << "HGR" << "DHGR" << "MONO";
    ui->comboBox_outputFormat->addItems(outputFormats);

    // POPULATE RESOLUTION COMBOBOX
    QStringList inputResolutions;
    inputResolutions << "40 x 48   - Full Scale LGR"
                << "80 x 48   - Full Scale DLGR"
                << "140 x 192 - Full Scale (HGR & DHGR)"
                << "280 x 192 - Double Width Scale (HGR & DHGR)"
                << "320 x 200 - Classic Size"
                << "560 x 384 - Quadruple Width, Double Height Scale"
                << "640 x 400 - Classic Size"
                << "640 x 480 - Classic Size";
    ui->comboBox_inputResolution->addItems(inputResolutions);
    // HANDLE DISPLAY MODE SELECTION (COMPATIBILITY)
    updateDisplayModes();

    // live preview stuff
    updateNeeded = false;
    previewTimer = new QTimer(this);
    connect(previewTimer, SIGNAL(timeout()), this, SLOT(previewTimerTimeout()));
}

//---- display modes
// 0  "40 x 48   - Full Scale LGR (LGR ONLY)"
// 1  "80 x 48   - Full Scale DLGR (DLGR ONLY)"
// 2  "140 x 192 - Full Scale (HGR & DHGR)"
// 3  "280 x 192 - Double Width Scale (HGR & DHGR)"
// 4  "320 x 200 - Classic Size"
// 5  "560 x 384 - Quadruple Width, Double Height Scale"
// 6  "640 x 400 - Classic Size"
// 7  "640 x 480 - Classic Size";
void MainWindow::updateDisplayModes() {

    QList<int> disabledList = QList<int>();

    // Enable all first
    QList<int> enabledList = QList<int>() << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7;
    QListIterator<int> e(enabledList);
    while (e.hasNext()) {
        // Get the index of the value to disable
        QModelIndex index = ui->comboBox_inputResolution->model()->index(e.next(), 0);
        // enable flag
        QVariant vEnable(1 | 32);
        // the magic
        ui->comboBox_inputResolution->model()->setData(index, vEnable, Qt::UserRole - 1);
    }

    // SET DISABLED ITEMS LIST - AND DEFAULT RESOLUTION FOR MODE
    if (ui->comboBox_outputFormat->currentText() == "LR") {
        disabledList << 2 << 3;
        inputWidth = 40;
        inputHeight = 48;
        ui->comboBox_inputResolution->setCurrentIndex(0);
    } else if (ui->comboBox_outputFormat->currentText() == "DLR") {
        disabledList << 2 << 3;
        inputWidth = 80;
        inputHeight = 48;
        ui->comboBox_inputResolution->setCurrentIndex(1);
    }  else if (ui->comboBox_outputFormat->currentText() == "MONO") {
        disabledList << 0 << 1 << 2 << 4 << 6 << 7;
        inputWidth = 280;
        inputHeight = 192;
        ui->comboBox_inputResolution->setCurrentIndex(3);
    } else if (ui->comboBox_outputFormat->currentText() == "DHGR") {
        inputWidth = 280;
        inputHeight = 192;
        ui->comboBox_inputResolution->setCurrentIndex(3);
    } else {
        inputWidth = 140;
        inputHeight = 192;
        ui->comboBox_inputResolution->setCurrentIndex(2);
    }

    // NOW ACTUALLY DISABLE INVALID RESOLUTIONS IN THE COMBOBOX
    QListIterator<int> d(disabledList);
    while (d.hasNext()) {
        // Get the index of the value to disable
        QModelIndex index = ui->comboBox_inputResolution->model()->index(d.next(), 0);
        // This is the effective 'disable' flag
        QVariant vDisable(0);
        // the magic
        ui->comboBox_inputResolution->model()->setData(index, vDisable, Qt::UserRole - 1);
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_sourceFilename_clicked()
{
    QString filename = QFileDialog::getOpenFileName();
    if (filename != NULL) {
        ui->lineEdit_sourceFilename->setText(filename);
        QPixmap mypix(filename);
        ui->label_source->setPixmap(mypix);
        ui->label_source->setScaledContents(true);
        QSize sourceSize = ui->label_source->pixmap()->size();
        QString resolutionString = QString("%1 x %2").arg(sourceSize.width()).arg(sourceSize.height());
        ui->label_sourceResolution->setText(resolutionString);
        updateNeeded=1;
    }
}

void MainWindow::updateInputSize()
{
    switch (ui->comboBox_inputResolution->currentIndex()) {
        case 0:
            inputWidth = 40;
            inputHeight = 48;
            break;
        case 1:
            inputWidth = 80;
            inputHeight = 48;
            break;
        case 2:
            inputWidth = 140;
            inputHeight = 192;
            break;
        case 3:
            inputWidth = 280;
            inputHeight = 192;
            break;
        case 4:
            inputWidth = 320;
            inputHeight = 200;
            break;
        case 5:
            inputWidth = 560;
            inputHeight = 384;
            break;
        case 6:
            inputWidth = 640;
            inputHeight = 400;
            break;
        case 7:
            inputWidth = 640;
            inputHeight = 480;
            break;
        default:
            qDebug() << "default, error?";
            break;
    }

    QSize sourceSize = ui->label_source->pixmap()->size();
    double sx = (double)inputWidth / sourceSize.width();
    double sy = (double)inputHeight / sourceSize.height();
    QString scaleString = QString("%1 x %2").arg(sx).arg(sy);
    ui->label_scaleFactor->setText(scaleString);
}

void MainWindow::livePreview()
{
    if (ui->checkBox_livePreview->isChecked()) {
        on_pushButton_preview_clicked();
    }
}


// This is the actual preview generation/main logic function
void MainWindow::on_pushButton_preview_clicked()
{
    if (ui->label_source->pixmap() == NULL) {
        ui->plainTextEdit_lastCmd->document()->setPlainText("Please open a source image first!");
        return;
    }
    // GET SCALE FACTOR
    updateInputSize();

    // NOW GENERATE SCALED QPIXMAP TO SAVE
    QPixmap scaledPixmap = ui->label_source->pixmap()->scaled(inputWidth,inputHeight);
    scaledPixmap.save(inputImgPath,"BMP", 0);

    // NOW FIND OUR OUTPUT FORMAT
    QString outputFormat = "H";     // HIRES
    switch (ui->comboBox_outputFormat->currentIndex()) {
        case 0:
            outputFormat = "L";
            break;
        case 1:
            outputFormat = "DL";
            break;
        case 2:
            outputFormat = "H";
            break;
        case 3:
            outputFormat = "D";
            break;
        case 4:
            outputFormat = "mono";
            break;
    }

    QString tempDir = "/tmp";


    QString converterPath = "/Users/dbrock/appleiigs/grlib/b2d";
    converterPath = QString("%1/b2d").arg(QCoreApplication::applicationDirPath());

    QProcess process;
    QStringList args;
    args << inputImgPath;   // "/tmp/saved.bmp"
    args << outputFormat;

    if (ui->horizontalSlider_crossHatch->value() > 0) {
        QString crossHatchArg = QString("Z%1").arg(ui->horizontalSlider_crossHatch->value());
        args << crossHatchArg;
    }
    if (ui->horizontalSlider_colorBleed->value() > 0) {
        QString colorBleedArg = QString("R%1").arg(ui->horizontalSlider_colorBleed->value());
        args << colorBleedArg;
    }

    args <<  "V";   // MUST HAVE!  OUR PREVIEW IMAGE

    // RUN THE CONVERTER SCRIPT
    process.start(converterPath,args);
    process.waitForFinished();  // BLOCKS!!!

    QString commandString = QString("%1 %2").arg(converterPath, args.join(" "));
    ui->plainTextEdit_lastCmd->document()->setPlainText(commandString);

    // ALL DONE SO TRY TO LOAD PREVIEW
    QPixmap previewPix(previewImgPath);
    float realScale = 1;
    if (previewPix.width() == 80) {
        int scale = 3;
        realScale = scale;
        previewPix = previewPix.scaled(80*scale, 48*scale);
    }
    if (previewPix.width() == 560) {
        float scale = 0.5f;
        realScale = scale;
        previewPix = previewPix.scaled((int)(560*scale),(int)(384*scale), Qt::KeepAspectRatio,Qt::SmoothTransformation);
    }
    ui->label_preview->setPixmap(previewPix);
    ui->groupBox_preview->setTitle(QString("Preview - Scale %1").arg(realScale));
}

// When this timer is running, it checks to see if a
// preview update has been requested, and handles it if so.
void MainWindow::previewTimerTimeout()
{
    if (updateNeeded) {
        on_pushButton_preview_clicked();
        updateNeeded = false;
    }
}

void MainWindow::on_horizontalSlider_crossHatch_valueChanged(int value)
{
    if (value==0){
        ui->label_crossHatch->setText("Off");
    } else {
        ui->label_crossHatch->setText(QString::number(value));
    }
    updateNeeded = 1;
}


void MainWindow::on_horizontalSlider_colorBleed_valueChanged(int value)
{
    if (value==0){
        ui->label_colorBleed->setText("Off");
    } else {
        ui->label_colorBleed->setText(QString::number(value));
    }
    updateNeeded = 1;
}

void MainWindow::on_comboBox_outputFormat_currentIndexChanged(int index)
{
    updateNeeded = 1;
    updateDisplayModes();
}


void MainWindow::on_comboBox_inputResolution_currentIndexChanged(int index)
{
    updateNeeded = 1;
}


void MainWindow::on_checkBox_livePreview_stateChanged(int arg1)
{
    // preview update timer
    if (arg1) {
        updateNeeded = 1;

        previewTimer->start(500);
    } else {
        previewTimer->stop();
    }
}


void MainWindow::on_actionWhat_is_this_triggered()
{
    QMessageBox msgBox;
    msgBox.setText("This is an image conversion utility to output images for use on classic 8-bit Apple II computers.\n\nPlease see readme for instructions.\n\nSorry for bugs, it's just a toy.\n\n(c)2016 Dagen Brock *\n\n\n * bmp2dhr is by Bill Buckles and does the the actual heavy lifting of conversion!  But don't bug him about this software, please.");
    msgBox.exec();
}

void MainWindow::on_pushButton_saveImage_clicked()
{
    if (ui->label_preview->pixmap() == NULL) {
        ui->plainTextEdit_lastCmd->document()->setPlainText("Please open a source image and run a preview first!");
        return;
    }
    QString a2filename;
    QString suffix;
    QString filters = QString("All Images (*.A2FC *.BIN *.SLO *.DLO);;HGR (*.BIN);;DHGR (*.A2FC);;LR (*.SLO);;DLR (*.DLO);;All files (*.*)");
    QString defaultFilter;

    if (ui->comboBox_outputFormat->currentText() == "LR") {
        a2filename = QString("%1/%2.SLO").arg(tmpDirPath,imageName.toUpper());
        suffix = ".SLO";
        defaultFilter = "LR (*.SLO)";
    } else if (ui->comboBox_outputFormat->currentText() == "DLR") {
        a2filename = QString("%1/%2.DLO").arg(tmpDirPath,imageName.toUpper());
        suffix = ".DLO";
        defaultFilter = "DLR (*.DLO)";
    } else if (ui->comboBox_outputFormat->currentText() == "HGR") {
        a2filename = QString("%1/%2CH.BIN").arg(tmpDirPath,imageName.toUpper());
        suffix = ".BIN";
        defaultFilter = "HGR (*.BIN)";
    } else if (ui->comboBox_outputFormat->currentText() == "DHGR") {
        a2filename = QString("%1/%2.A2FC").arg(tmpDirPath,imageName.toUpper());
        suffix = ".A2FC";
        defaultFilter = "DHGR (*.A2FC)";
    } else if (ui->comboBox_outputFormat->currentText() == "MONO") {
        a2filename = QString("%1/%2M.BIN").arg(tmpDirPath,imageName.toUpper());
        suffix = ".BIN";
        defaultFilter = "HGR (*.BIN)";
    }

    // PROMPT FOR SAVE FILENAME AND COPY (HOPEFULLY) TO SAVE FILENAME
    QString saveFile = QFileDialog::getSaveFileName(0, "Save file", QDir::currentPath(), filters, &defaultFilter);
    QFile::copy(a2filename, saveFile);
}


// HOLY CRAP WHAT IS THIS??
// Well you see, Johnny, this is a layer of hacks to try
// to wrap CADIUS, but it has no API.  So I can clean this up
// but it's more of a proof of concept.  I think it'd be
// smarter to add JSON output to CADIUS in the long run.
// I consider this small feature a present to the community.
void MainWindow::on_pushButton_saveToProdos_clicked()
{
    // @Todo: This isn't appropriate when someone changes res/source and has
    // a previous preview pixmap, it will think we are all OK.
    if (ui->label_preview->pixmap() == NULL) {
        ui->plainTextEdit_lastCmd->document()->setPlainText("Please open a source image and run a preview first!");
        return;
    }

    QString cadiusPath = "/Users/dbrock/appleiigs/tools/Cadius";
    cadiusPath = QString("%1/Cadius").arg(QCoreApplication::applicationDirPath());


    // KSYNTHED=Type(06),AuxType(2000),VersionCreate(70),MinVersion(BE),Access(E3),FolderInfo1(000000000000000000000000000000000000),FolderInfo2(000000000000000000000000000000000000)
    QString filetype = "06";
    QString auxtype = "2000";


    QString suffix = ".po";
    QString defaultFilter = tr("All ProDOS Images (*.po *.hdv *.2mg)");
    QString filters = QString(tr("All ProDOS Images (*.po *.hdv *.2mg);;ProDOS Order (*.po);;HDV (*.hdv);;2MG (*.2mg);;All files (*.*)"));


    // PROMPT FOR SAVE FILENAME AND COPY (HOPEFULLY) TO SAVE FILENAME
    QString prodosImageFile = QFileDialog::getSaveFileName(0, "Choose ProDOS Image to Save to", QDir::currentPath(), filters, &defaultFilter, QFileDialog::DontConfirmOverwrite);

    // ALSO GENERATE PRODOS SAFE BASENAME
    QFileInfo fi(prodosImageFile);
    QString prodosVolumeName = fi.baseName().left(15);  // get max volume name
    // EMPTY FILENAME?!
    if (prodosImageFile.length() == 0) {
        return;
    }

    QString imageSize = "140KB";

    // NOW SEE IF IT'S A NEW FILE OR EXISTING IMAGE
    QFileInfo check_file(prodosImageFile);
    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile()) {
        // nothing yet.  We'll open it below, either way.
    } else {
        // NEW FILE, PROMPT FOR IMAGE SIZE
        QMessageBox msgBox;
        msgBox.setText(tr("Select size for new ProDOS image"));
        msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
        QAbstractButton* pButton140 = msgBox.addButton(tr("140KB"), QMessageBox::ApplyRole);
        QAbstractButton* pButton800 = msgBox.addButton(tr("800KB"), QMessageBox::ApplyRole);
        msgBox.setDefaultButton(QMessageBox::Yes);

        msgBox.exec();

        if (msgBox.clickedButton()==pButton140) {
            // set above, nothing to do
        } else if (msgBox.clickedButton()==pButton800) {
            imageSize = "800KB";
        } else {
            return; // cancelled
        }

        // NOW CREATE AN IMAGE
        QProcess process;
        QStringList args;
        args << "CREATEVOLUME";
        args << prodosImageFile;
        args << prodosVolumeName;
        args << imageSize;
        QString commandString = QString("%1 %2").arg(cadiusPath, args.join(" "));

        // RUN THE IMAGE CREATE
        process.start(cadiusPath,args);
        process.waitForFinished();  // BLOCKS!!!

        // NOW CHECK AGAIN TO SEE IF OUR IMAGE FILE GOT CREATED
        if (check_file.exists() && check_file.isFile()) {
            qDebug() << "IMAGE FILE CREATED.";
        } else {
            ui->plainTextEdit_lastCmd->document()->setPlainText(QString("Failed creating image with command: %1").arg(commandString));
            return;
        }

        ui->plainTextEdit_lastCmd->document()->setPlainText(commandString);
    }

    // NOW CATALOG WHATEVER IMAGE WE GOT... NEW/EXISTING
    QProcess cat_process;
    QStringList cat_args;
    cat_args << "CATALOG";
    cat_args << prodosImageFile;

    // RUN THE CATALOG AND GET OUTPUT
    cat_process.start(cadiusPath,cat_args);
    cat_process.waitForFinished();  // BLOCKS!!!
    QString cat_output = QString(cat_process.readAllStandardOutput());
    qDebug() << "CATALOG OUTPUT (cat_output)\n   " << cat_output;

    // regex scanner index
    int pos = 0;
    QStringList list;

    // MUST MATCH FOR NEWLINES (VS USING ^ or &)
    QRegExp vol_rx("\n(/.{1,15}/)\n");
    // OVERWRITE PRODOS VOLUME NAME IF WE KNOW BETTER
    while ((pos = vol_rx.indexIn(cat_output, pos)) != -1) {
        prodosVolumeName = vol_rx.cap(1);
        break;
    }

    // NOW MATCH FOR SOME EXTRA DETAILS JUST BECAUSE WE CAN
    QRegExp rx("(Block|Free|File|Directory) : (\\d+)");
    pos = 0;

    while ((pos = rx.indexIn(cat_output, pos)) != -1) {
        list << rx.cap(1);
        qDebug() << rx.cap(1);
        pos += rx.matchedLength();
    }

    int diskBlocks, diskFree, diskFiles, diskDirs = 0;
    if (list.length() == 4) {
        diskBlocks = list[0].toInt();
        diskFree = list[1].toInt();
        diskFiles = list[2].toInt();
        diskDirs = list[3].toInt();
    } else {
        ui->plainTextEdit_lastCmd->document()->setPlainText(QString("Failed to catalog ProDOS volume. Make sure that the file is one of: .po .hdv .2mg"));
        return;
    }



    QString a2Filename;
    QString savedFilename;
    if (ui->comboBox_outputFormat->currentText() == "LR") {
        savedFilename = QString("%1/%2.SLO").arg(tmpDirPath,imageName.toUpper());
        a2Filename = QString("%1.SLO").arg(imageName.toUpper());
        auxtype = "400";    // different auxtype (not that is matters)
    } else if (ui->comboBox_outputFormat->currentText() == "DLR") {
        savedFilename = QString("%1/%2.DLO").arg(tmpDirPath,imageName.toUpper());
        a2Filename = QString("%1.DLO").arg(imageName.toUpper());
        auxtype = "400";    // different auxtype (not that is matters)
    } else if (ui->comboBox_outputFormat->currentText() == "HGR") {
        savedFilename = QString("%1/%2CH.BIN").arg(tmpDirPath,imageName.toUpper());
        a2Filename = QString("%1CH.BIN").arg(imageName.toUpper());
    } else if (ui->comboBox_outputFormat->currentText() == "DHGR") {
        savedFilename = QString("%1/%2.A2FC").arg(tmpDirPath,imageName.toUpper());
        a2Filename = QString("%1.A2FC").arg(imageName.toUpper());
    } else if (ui->comboBox_outputFormat->currentText() == "MONO") {
        savedFilename = QString("%1/%2M.BIN").arg(tmpDirPath,imageName.toUpper());
        a2Filename = QString("%1M.BIN").arg(imageName.toUpper());
    }


    bool ok;
    QString prodosFileName = QInputDialog::getText(this, tr("Save Image to ProDOS as"),
                                         tr("ProDOS Name (max 15 chars):"), QLineEdit::Normal,
                                         a2Filename, &ok);
    if (ok && !prodosFileName.isEmpty()) {
        // COPY IT ...  OVER EXISTING NAME?
        QString saveFile = QString("%1/%2").arg(tmpDirPath,prodosFileName);
        QFile::copy(savedFilename, saveFile);

        // GENERATE OUR STUPID _FileInformation.txt in the same directory
        QString fileinfo_text = QString("%1=Type(%2),AuxType(%3),VersionCreate(70),MinVersion(BE),Access(E3),FolderInfo1(000000000000000000000000000000000000),FolderInfo2(000000000000000000000000000000000000)").arg(prodosFileName, filetype, auxtype);
        QString fileinfo_file = QString("%1/_FileInformation.txt").arg(tmpDirPath);
        qDebug() << "TMP FILE: " << fileinfo_file;
        QFile file( fileinfo_file );
        if (file.open(QIODevice::ReadWrite)) {
            QTextStream stream( &file );
            stream << fileinfo_text << endl;
        }

        // NOW ADD / SAVE OUR FILE
        QProcess addfile_process;
        QStringList addfile_args;
        addfile_args << "ADDFILE" << prodosImageFile << prodosVolumeName << saveFile;   // our tmp file

        addfile_process.start(cadiusPath,addfile_args);
        addfile_process.waitForFinished();  // BLOCKS!!!
        QString addfile_output = QString(addfile_process.readAllStandardOutput());

        // IF FILE ALREADY EXISTS, ASK IF THEY WANT TO OVERWRITE
        if (addfile_output.contains("A file already exist with the same nam")) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "File exists in image", "File exists, Replace?",
                                        QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {

                // ../tools/Cadius DELETEFILE    <[2mg|hdv|po]_image_path>   <prodos_file_path>
                // YES - DELETE
                QString deleteFile = QString("%1%2").arg(prodosVolumeName,prodosFileName);
                qDebug() << "DELETEFILE : " <<deleteFile;
                QProcess delfile_process;
                QStringList delfile_args;
                delfile_args << "DELETEFILE";
                delfile_args << prodosImageFile;
                delfile_args << deleteFile;

                // NOW ADD / SAVE OUR FILE
                delfile_process.start(cadiusPath,delfile_args);
                delfile_process.waitForFinished();  // BLOCKS!!!
                QString delfile_output = QString(delfile_process.readAllStandardOutput());
                // MAYBE CHECK? BUT WE CAN JUST TRY TO RESAVE AND FAIL THERE

                // TRY ADDFILE AGAIN
                QProcess addfile2_process;
                QStringList addfile2_args;
                addfile2_args << "ADDFILE" << prodosImageFile << prodosVolumeName << saveFile;   // our tmp file

                addfile2_process.start(cadiusPath,addfile_args);
                addfile2_process.waitForFinished();  // BLOCKS!!!
                QString addfile2_output = QString(addfile2_process.readAllStandardOutput());

                // IF FILE ALREADY EXISTS, ASK IF THEY WANT TO OVERWRITE
                if (addfile2_output.contains("Error :")) {
                    ui->plainTextEdit_lastCmd->document()->setPlainText("Save failed.  Couldn't delete/overwrite file?  I really am not sure what went wrong.  Try saving the pictures to your computer and transferring them to ProDOS with another program.");
                    return;
                }
            } else {
                if (ui->label_preview->pixmap() == NULL) {
                    ui->plainTextEdit_lastCmd->document()->setPlainText("Save cancelled because file exists.");
                    return;
                }
            }
        }


        // IF WE MADE IT THIS FAR, ALL GOOD?
        QFile::remove(saveFile);    // remove our tmp file
        ui->plainTextEdit_lastCmd->document()->setPlainText("File saved.");

    } else {
        ui->plainTextEdit_lastCmd->document()->setPlainText("Save cancelled.");
        // cancelled?
    }


    return;
}
