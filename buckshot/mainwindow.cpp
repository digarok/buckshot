#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qfiledialog.h"
#include "qprocess.h"
#include "qdebug.h"
#include "qtimer.h"
#include "qmessagebox.h"
#include "qinputdialog.h"
#include "qformlayout.h"
#include "qdialogbuttonbox.h"
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
const QString MainWindow::programName = QString("buckshot");
const QString MainWindow::version = QString("0.8.0");
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
    shrPreviewImgPath = QString("%1/shr_preview.png").arg(tmpDirPath);

    // POPULATE FORMAT COMBOBOX
    ui->comboBox_outputFormat->addItems(modeNamesForEngine(Engine::B2D));

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

    // POPULATE DITHERING COMBOBOX
    QStringList ditheringAlgorithms;
    ditheringAlgorithms << "Default"
                        << "1- Floyd-Steinberg"
                        << "2- Jarvis"
                        << "3- Stucki"
                        << "4- Atkinson"
                        << "5- Burkes"
                        << "6- Sierra"
                        << "7- Sierra Two"
                        << "8- Sierra Lite"
                        << "9- Buckels";
    ui->comboBox_dithering->addItems(ditheringAlgorithms);


    // POPULATE PALETTE COMBOBOX
    QStringList previewPalettes;
    previewPalettes << "Kegs32 RGB"
            << "CiderPress RGB"
            << "Old AppleWin NTSC"
            << "New AppleWin NTSC"
            << "Wikipedia NTSC"
            << "tohgr NTSC DHGR"
            << " (N/A) -- Imported"
            << "Legacy Canvas"
            << "Legacy Win16"
            << "Legacy Win32"
            << "Legacy VGA BIOS"
            << "Legacy VGA PCX"
            << "Super Convert RGB"
            << "Jace NTSC"
            << "Cybernesto-Munafo NTSC"
            << "Pseudo Palette"
            << "tohgr NTSC HGR";
    ui->comboBox_previewPalette->addItems(previewPalettes);

    // POPULATE THE IIgs SHR (image2shr) TAB
    // Display text is friendly; the CLI token rides along as item data.
    ui->comboBox_shrTarget->addItems(modeNamesForEngine(Engine::Image2SHR));

    ui->comboBox_shrDither->addItem("None", "none");
    ui->comboBox_shrDither->addItem("Floyd-Steinberg", "floyd-steinberg");
    ui->comboBox_shrDither->addItem("Atkinson", "atkinson");
    ui->comboBox_shrDither->addItem("Jarvis", "jarvis");
    ui->comboBox_shrDither->addItem("Sierra", "sierra");
    ui->comboBox_shrDither->addItem("Bayer 2x2", "bayer2");
    ui->comboBox_shrDither->addItem("Bayer 4x4", "bayer4");
    ui->comboBox_shrDither->addItem("Bayer 8x8", "bayer8");

    ui->comboBox_shrFit->addItem("Contain", "contain");
    ui->comboBox_shrFit->addItem("Cover", "cover");
    ui->comboBox_shrFit->addItem("Stretch", "stretch");
    ui->comboBox_shrFit->addItem("None", "none");

    ui->comboBox_shrAspect->addItem("Correct", "correct");
    ui->comboBox_shrAspect->addItem("Ignore", "ignore");

    ui->comboBox_shrScbMode->addItem("Auto", "auto");
    ui->comboBox_shrScbMode->addItem("Single", "single");
    ui->comboBox_shrScbMode->addItem("Banded", "banded");
    ui->comboBox_shrScbMode->addItem("Grouped", "grouped");
    ui->comboBox_shrScbMode->addItem("Per-Line", "per-line");

    ui->comboBox_shrFormat->addItem("Auto", "auto");
    ui->comboBox_shrFormat->addItem("Raw", "raw");
    ui->comboBox_shrFormat->addItem("Packed", "packed");
    ui->comboBox_shrFormat->addItem("APF", "apf");
    ui->comboBox_shrFormat->addItem("Brooks 3200", "brooks");

    // MATCH THE image2shr CLI DEFAULTS, BUT START ON A COLOR MODE
    // AND STRETCH-TO-FIT LIKE THE b2d PIPELINE DOES
    ui->comboBox_shrTarget->setCurrentText("SHR 320 Color 256");
    ui->comboBox_shrDither->setCurrentText("Floyd-Steinberg");
    ui->comboBox_shrFit->setCurrentText("Stretch");
    updateShrControls();

    // HANDLE DISPLAY MODE SELECTION (COMPATIBILITY)
    updateDisplayModes();

    // DEFAULT TO HGR AT 640 x 480 ON LAUNCH
    ui->comboBox_outputFormat->setCurrentText("HGR");
    ui->comboBox_inputResolution->setCurrentIndex(7);

    // live preview stuff
    updateNeeded = false;
    previewTimer = new QTimer(this);
    connect(previewTimer, SIGNAL(timeout()), this, SLOT(previewTimerTimeout()));
    // decided to enable by checking in the form and calling this.
    on_checkBox_livePreview_stateChanged(1);
}


MainWindow::~MainWindow()
{
    delete ui;
}


// The active display mode's descriptor: the single source of truth for
// converter arguments, output filenames, and ProDOS metadata.
// The selected tab picks the engine.
const ModeDescriptor &MainWindow::currentMode() const
{
    const ModeDescriptor *mode = nullptr;
    if (ui->tabWidget_engine->currentWidget() == ui->tab_shr) {
        mode = findMode(ui->comboBox_shrTarget->currentText());
    } else {
        mode = findMode(ui->comboBox_outputFormat->currentText());
    }
    return mode ? *mode : allModes().first();
}


bool MainWindow::check_canPreview()
{
    if (sourcePixmap.isNull()) {
        ui->plainTextEdit_lastCmd->setPlainText("Please open a source image to run a preview!");
        repaint();
        return false;
    }
    return true;
}

bool MainWindow::check_canSave()
{
    if (ui->label_preview->pixmap().isNull()) {
        ui->plainTextEdit_lastCmd->setPlainText("Please open a source image and run a preview first!");
        repaint();
        return false;
    }
    // TWO ENGINES SHARE ONE PREVIEW — REFUSE TO SAVE A STALE RESULT
    const ModeDescriptor &mode = currentMode();
    if (lastPreviewEngine != mode.engine || lastPreviewModeName != mode.name) {
        ui->plainTextEdit_lastCmd->setPlainText("Settings changed since the last preview - run a preview first!");
        repaint();
        return false;
    }
    return true;
}


void MainWindow::updateDisplayModes() {

    const ModeDescriptor &mode = currentMode();
    QList<int> disabledList = mode.disabledResolutions;

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

    // KEEP THE CURRENT RESOLUTION WHEN THE NEW MODE STILL SUPPORTS IT,
    // OTHERWISE FALL BACK TO THE MODE'S DEFAULT
    if (disabledList.contains(ui->comboBox_inputResolution->currentIndex())) {
        ui->comboBox_inputResolution->setCurrentIndex(mode.defaultResolution);
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


void MainWindow::on_pushButton_sourceFilename_clicked()
{
    QString filename = QFileDialog::getOpenFileName();
    if (!filename.isEmpty()) {
        ui->lineEdit_sourceFilename->setText(filename);
        QPixmap mypix(filename);
        sourcePixmap = mypix;
        // FILL THE PANE UNTIL THE FIRST PREVIEW RESIZES IT TO MATCH
        ui->label_source->setScaledContents(false);
        ui->label_source->setPixmap(sourcePixmap.scaled(ui->label_source->size(),
                                                        Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        QSize sourceSize = mypix.size();
        QString resolutionString = QString("%1 x %2").arg(sourceSize.width()).arg(sourceSize.height());
        ui->label_sourceResolution->setText(resolutionString);
        updateNeeded=1;
    }
}


// FILE > OPEN SOURCE IMAGE - SAME DIALOG AS THE BUTTON
void MainWindow::on_actionOpen_Source_Image_triggered()
{
    on_pushButton_sourceFilename_clicked();
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

}


// THE "OUTPUT RESOLUTION" AND "SCALE" READOUTS NEXT TO THE SOURCE
// RESOLUTION: WHAT THE CONVERTER PRODUCED, AND OUTPUT SIZE OVER SOURCE
// SIZE, PER AXIS
void MainWindow::updateScaleLabel(int outWidth, int outHeight)
{
    ui->label_outputResolution->setText(QString("%1 x %2").arg(outWidth).arg(outHeight));

    QSize sourceSize = sourcePixmap.size();
    double sx = static_cast<double>(outWidth) / sourceSize.width();
    double sy = static_cast<double>(outHeight) / sourceSize.height();
    QString scaleString = QString("%1 x %2").arg(sx, 0, 'f', 2).arg(sy, 0, 'f', 2);
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
    if (!check_canPreview()) return;

    const ModeDescriptor &mode = currentMode();
    if (mode.engine == Engine::Image2SHR) {
        runShrConversion(mode);
    } else {
        runB2dConversion(mode);
    }
}


void MainWindow::runB2dConversion(const ModeDescriptor &mode)
{
    // GET SCALE FACTOR
    updateInputSize();

    // NOW GENERATE SCALED QPIXMAP TO SAVE
    QPixmap scaledPixmap = sourcePixmap.scaled(inputWidth,inputHeight);
    scaledPixmap.save(inputImgPath,"BMP", 0);

    // BUILD THE CONVERTER COMMAND FROM THE MODE DESCRIPTOR
    ConversionParams params;
    params.inputPath = inputImgPath;   // "/tmp/saved.bmp"
    params.crossHatch = ui->horizontalSlider_crossHatch->value();
    params.colorBleed = ui->horizontalSlider_colorBleed->value();
    params.ditherIndex = ui->comboBox_dithering->currentIndex();
    // MUST ALWAYS HAVE A "V" FLAG TO GENERATE OUR PREVIEW IMAGE
    params.paletteIndex = ui->comboBox_previewPalette->currentIndex();
    params.extraArgsText = ui->lineEdit_addArgs->text();

    QString converterPath = QString("%1/b2d").arg(QCoreApplication::applicationDirPath());

    QProcess process;
    QStringList args = mode.buildArgs(mode, params);

    // RUN THE CONVERTER SCRIPT
    process.start(converterPath,args);
    process.waitForFinished();  // BLOCKS!!!

    QString commandString = QString("%1 %2").arg(converterPath, args.join(" "));
    if (process.error() == QProcess::FailedToStart) {
        ui->plainTextEdit_lastCmd->document()->setPlainText(
            QString("b2d not found at %1 - it should sit next to the buckshot executable.").arg(converterPath));
        repaint();
        return;
    }
    //qDebug() << commandString;
    ui->plainTextEdit_lastCmd->document()->setPlainText(commandString);

    // ALL DONE SO TRY TO LOAD PREVIEW
    QPixmap previewPix(previewImgPath);
    showPreviewAndSource(previewPix);

    // SCALE READOUT COMPARES THE SOURCE TO WHAT THE CONVERTER ACTUALLY
    // PRODUCED, NOT TO THE INTERMEDIATE INPUT RESOLUTION
    if (!previewPix.isNull()) {
        updateScaleLabel(previewPix.width(), previewPix.height());
    }

    lastPreviewEngine = mode.engine;
    lastPreviewModeName = mode.name;
    repaint();
}


// SHOW THE PREVIEW AT THE LARGEST PIXEL-PERFECT INTEGER SCALE THAT FITS
// THE PANE, AND SHOW THE SOURCE AT EXACTLY THE SAME DISPLAYED SIZE SO THE
// TWO COMPARE 1:1.
void MainWindow::showPreviewAndSource(const QPixmap &previewPix)
{
    if (previewPix.isNull()) {
        return;
    }
    QSize pane = ui->label_preview->size();
    int scale = qMax(1, qMin(pane.width() / previewPix.width(),
                             pane.height() / previewPix.height()));
    QSize displaySize = previewPix.size() * scale;

    // FastTransformation keeps the upscaled pixels crisp
    ui->label_preview->setPixmap(previewPix.scaled(displaySize));
    // THE PANES NO LONGER HAVE TITLES, SO THE SCALE READOUT GOES TO THE LOG
    ui->plainTextEdit_lastCmd->appendPlainText(QString("Preview scale: %1x").arg(scale));

    // THE CONVERTERS MAP THE FULL SOURCE ONTO THE OUTPUT, SO STRETCH LIKEWISE
    ui->label_source->setPixmap(sourcePixmap.scaled(displaySize,
                                                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}


void MainWindow::runShrConversion(const ModeDescriptor &mode)
{
    // EVERY SHR TARGET RENDERS THE 320 x 200 SCREEN - KEEP THE SCALE
    // READOUT LIVE JUST LIKE THE b2d PATH DOES VIA updateInputSize()
    updateScaleLabel(320, 200);

    ConversionParams params;

    // FEED image2shr THE ORIGINAL FILE WHEN IT CAN READ IT DIRECTLY -
    // ITS OWN FIT/ASPECT RESAMPLING FROM FULL RESOLUTION BEATS A
    // PRE-SCALED BMP. OTHERWISE DUMP THE SOURCE PIXMAP UNSCALED.
    QString sourceFile = ui->lineEdit_sourceFilename->text();
    QFileInfo sourceInfo(sourceFile);
    static const QStringList shrReadableFormats = {"png", "jpg", "jpeg", "gif", "bmp"};
    if (sourceInfo.isFile() && shrReadableFormats.contains(sourceInfo.suffix().toLower())) {
        params.inputPath = sourceFile;
    } else {
        params.inputPath = QString("%1/source_full.png").arg(tmpDirPath);
        sourcePixmap.save(params.inputPath, "PNG");
    }

    // .3200 WHENEVER THE EFFECTIVE OUTPUT IS BROOKS FORMAT
    QString format = ui->comboBox_shrFormat->currentData().toString();
    bool brooks = (format == "brooks")
            || (format == "auto" && mode.shrTarget == "shr320-color3200");
    shrOutputPath = QString("%1/%2").arg(tmpDirPath, brooks ? "SAVED.3200" : "SAVED.SHR");

    params.outputPath = shrOutputPath;
    params.previewPath = shrPreviewImgPath;
    params.extraArgsText = ui->lineEdit_addArgs->text();
    params.shrDither = ui->comboBox_shrDither->currentData().toString();
    // SLIDER IS 0-100; image2shr TAKES 0.0-1.0
    params.shrDitherStrength = ui->horizontalSlider_shrDitherStrength->value() / 100.0;
    params.shrSerpentine = ui->checkBox_shrSerpentine->isChecked();
    params.shrFit = ui->comboBox_shrFit->currentData().toString();
    params.shrAspect = ui->comboBox_shrAspect->currentData().toString();
    // OMITTED ENTIRELY FOR TARGETS WITH NO SCB CHOICE - THEY REJECT THE FLAG
    params.shrScbMode = mode.shrHasScbModes
            ? ui->comboBox_shrScbMode->currentData().toString() : QString();
    params.shrFormat = format;

    QString converterPath = QString("%1/image2shr").arg(QCoreApplication::applicationDirPath());
    QStringList args = mode.buildArgs(mode, params);
    QString commandString = QString("%1 %2").arg(converterPath, args.join(" "));

    QProcess process;
    process.start(converterPath, args);
    bool finished = process.waitForFinished(20000);

    if (process.error() == QProcess::FailedToStart) {
        ui->plainTextEdit_lastCmd->document()->setPlainText(
            QString("image2shr not found at %1 - download it from github.com/digarok/image2shr/releases "
                    "and place it next to the buckshot executable.").arg(converterPath));
        repaint();
        return;
    }
    if (!finished) {
        process.kill();
        process.waitForFinished(2000);
        ui->plainTextEdit_lastCmd->document()->setPlainText(
            QString("image2shr timed out after 20 seconds: %1").arg(commandString));
        repaint();
        return;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        QString stderrText = QString(process.readAllStandardError()).trimmed();
        ui->plainTextEdit_lastCmd->document()->setPlainText(
            QString("image2shr failed (exit %1): %2\n%3")
                .arg(process.exitCode()).arg(commandString, stderrText));
        repaint();
        return;
    }

    // PARSE THE --json REPORT: ProDOS TYPE/AUX (TRACKS --format) AND WARNINGS
    shrJsonFileType.clear();
    shrJsonAuxType.clear();
    QString logText = commandString;
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(process.readAllStandardOutput(), &jsonError);
    if (jsonError.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject result = doc.object();
        QJsonObject prodos = result.value("prodos").toObject();
        // "$C1"/"$0000" -> "C1"/"0000" as cadius wants them
        shrJsonFileType = prodos.value("file_type_hex").toString().remove('$');
        shrJsonAuxType = prodos.value("aux_type_hex").toString().remove('$');
        const QJsonArray warnings = result.value("warnings").toArray();
        for (const QJsonValue &warning : warnings) {
            logText += QString("\nWarning: %1").arg(warning.toString());
        }
    }
    ui->plainTextEdit_lastCmd->document()->setPlainText(logText);

    showPreviewAndSource(QPixmap(shrPreviewImgPath));

    lastPreviewEngine = mode.engine;
    lastPreviewModeName = mode.name;
    repaint();
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


void MainWindow::on_comboBox_outputFormat_currentIndexChanged(int /*unused*/)
{
    updateNeeded = 1;
    updateDisplayModes();
}


void MainWindow::on_comboBox_inputResolution_currentIndexChanged(int /*unused*/)
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


void MainWindow::on_actionAbout_Buckshot_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("About Buckshot");
    msgBox.setIconPixmap(QPixmap(":/icons/appicon.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.setText(QString("%1 v%2\n").arg(programName, version) +
                   "Image conversion tool for macOS/Windows/Linux\n"
                   "Convert modern image formats (PNG, JPEG, BMP, etc) into native Apple II and Apple IIgs graphics for your programs, or just for fun.\n\n\n"
                   "*USAGE*\n"
                   "1. Click \"Open Source Image\" and pick a picture.\n\n"
                   "2. Choose a display mode: the \"8-bit (b2d)\" tab covers the classic Apple II/II+/IIe/IIc modes (HGR, DHGR, LR, DLR), "
                   "and the \"IIgs SHR (image2shr)\" tab handles Apple IIgs Super Hi-Res conversions, including Brooks 3200-color output.\n\n"
                   "3. Click \"Preview\", or check \"Live Preview\" for real-time feedback as you adjust conversion settings.\n\n"
                   "4. Click \"Save Image File\" to save in the Apple ][ image format for that display mode, "
                   "or use \"Save To ProDOS\" to write it directly onto a ProDOS volume.\n\n"
                   "Then load it up in your favorite emulator, or transfer it to real disks/flash storage to view on glorious vintage hardware.\n\n"
                   "(c)2016-2026 Dagen Brock *\n\n\n * bmp2dhr is by Bill Buckels, CADIUS is by Brutal Deluxe, and image2shr is by Dagen Brock.");
    msgBox.exec();
}


void MainWindow::on_pushButton_saveImage_clicked()
{
    if (!check_canSave()) return;

    const ModeDescriptor &mode = currentMode();
    QString a2filename = QString("%1/%2").arg(tmpDirPath, mode.outputFileName);
    QString filters = QString("All Images (*.A2FC *.A2FM *.BIN *.SLO *.DLO *.SHR *.3200);;HGR (*.BIN);;DHGR (*.A2FC);;DHGR MONO (*.A2FM);;LR (*.SLO);;DLR (*.DLO);;SHR (*.SHR);;Brooks 3200 (*.3200);;All files (*.*)");
    QString defaultFilter = mode.saveFilter;

    // SHR OUTPUT NAME CAN DIFFER FROM THE TABLE WHEN --format OVERRIDES IT
    if (mode.engine == Engine::Image2SHR && !shrOutputPath.isEmpty()) {
        a2filename = shrOutputPath;
        defaultFilter = shrOutputPath.endsWith(".3200") ? "Brooks 3200 (*.3200)" : "SHR (*.SHR)";
    }

    // PROMPT FOR SAVE FILENAME AND COPY (HOPEFULLY) TO SAVE FILENAME
    QString saveFile = QFileDialog::getSaveFileName(nullptr, "Save file", QDir::currentPath(), filters, &defaultFilter);
    if (QFile::exists(saveFile)) {
        QFile::remove(saveFile);
    }
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
    if (!check_canSave()) return;

    QString cadiusPath = "/Users/dbrock/appleiigs/cadius/cadius"; // Dev mode 
    cadiusPath = QString("%1/cadius").arg(QCoreApplication::applicationDirPath());


    // MYPICBIN=Type(06),AuxType(2000),VersionCreate(70),MinVersion(BE),Access(E3),FolderInfo1(000000000000000000000000000000000000),FolderInfo2(000000000000000000000000000000000000)
    const ModeDescriptor &mode = currentMode();
    QString filetype = mode.prodosFileType;
    QString auxtype = mode.prodosAuxType;

    // FOR SHR, PREFER WHAT image2shr --json REPORTED - IT TRACKS --format
    // VARIANTS (raw/packed/apf/brooks) WITHOUT US HARDCODING IIgs LORE
    if (mode.engine == Engine::Image2SHR) {
        if (!shrJsonFileType.isEmpty()) filetype = shrJsonFileType;
        if (!shrJsonAuxType.isEmpty()) auxtype = shrJsonAuxType;
    }


    QString suffix = ".po";
    QString defaultFilter = tr("All ProDOS Images (*.po *.hdv *.2mg)");
    QString filters = QString(tr("All ProDOS Images (*.po *.hdv *.2mg);;ProDOS Order (*.po);;HDV (*.hdv);;2MG (*.2mg);;All files (*.*)"));


    // PROMPT FOR SAVE FILENAME AND COPY (HOPEFULLY) TO SAVE FILENAME
    QString prodosImageFile = QFileDialog::getSaveFileName(nullptr, "Choose ProDOS Image to Save to", QDir::currentPath(), filters, &defaultFilter, QFileDialog::DontConfirmOverwrite);

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
        QAbstractButton* pButton32768 = msgBox.addButton(tr("32MB"), QMessageBox::ApplyRole);
        msgBox.setDefaultButton(QMessageBox::Yes);

        msgBox.exec();

        if (msgBox.clickedButton()==pButton140) {
            // set above, nothing to do
        } else if (msgBox.clickedButton()==pButton800) {
            imageSize = "800KB";
        } else if (msgBox.clickedButton()==pButton32768) {
            imageSize = "32MB";
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
            // qDebug() << "IMAGE FILE CREATED.";
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
    //qDebug() << "CATALOG OUTPUT (cat_output)\n   " << cat_output;

    QStringList list;

    // MUST MATCH FOR NEWLINES (VS USING ^ or &)
    QRegularExpression vol_rx("\n(/.{1,15}/)\r?\n");
    // OVERWRITE PRODOS VOLUME NAME IF WE KNOW BETTER
    QRegularExpressionMatch vol_match = vol_rx.match(cat_output);
    if (vol_match.hasMatch()) {
        prodosVolumeName = vol_match.captured(1);
    }

    // NOW MATCH FOR SOME EXTRA DETAILS JUST BECAUSE WE CAN
    QRegularExpression rx("(Block|Free|File|Directory) : (\\d+)");
    QRegularExpressionMatchIterator rx_it = rx.globalMatch(cat_output);

    while (rx_it.hasNext()) {
        list << rx_it.next().captured(1);
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



    QString a2Filename = mode.outputFileName;
    QString savedFilename = QString("%1/%2").arg(tmpDirPath, a2Filename);
    if (mode.engine == Engine::Image2SHR && !shrOutputPath.isEmpty()) {
        savedFilename = shrOutputPath;
        a2Filename = QFileInfo(shrOutputPath).fileName();
    }


    bool ok = false;
    QString prodosFileName;


    // manually build name/auxtype dialog
    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    // Add some text above the fields
    form.addRow(new QLabel(tr("Save Image to ProDOS  (filetype $%1)").arg(filetype)));

    // Add the lineEdits with their respective labels
    QList<QLineEdit *> fields;

    QLineEdit *lineEdit = new QLineEdit(&dialog);
    lineEdit->setText(a2Filename);
    QString label = QString("ProDOS Name (max 15 chars):");
    form.addRow(label, lineEdit);

    QLineEdit *lineEdit2 = new QLineEdit(&dialog);
    lineEdit2->setText(auxtype);
    lineEdit2->setInputMask("HHHh");
    QString label2 = QString("Change auxtype (optional): $");
    form.addRow(label2, lineEdit2);

    fields << lineEdit << lineEdit2;


    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted) {
        // If the user didn't dismiss the dialog, do something with the fields
        prodosFileName = fields[0]->text();
        auxtype = fields[1]->text();
        ok = true;
    }


    if (ok && !prodosFileName.isEmpty()) {
        // COPY IT ...  OVER EXISTING NAME?
        QString saveFile = QString("%1/%2").arg(tmpDirPath,prodosFileName);
        QFile::copy(savedFilename, saveFile);

        // GENERATE OUR STUPID _FileInformation.txt in the same directory
        QString fileinfo_text = QString("%1=Type(%2),AuxType(%3),VersionCreate(70),MinVersion(BE),Access(E3),FolderInfo1(000000000000000000000000000000000000),FolderInfo2(000000000000000000000000000000000000)").arg(prodosFileName, filetype, auxtype);
        QString fileinfo_file = QString("%1/_FileInformation.txt").arg(tmpDirPath);
        //qDebug() << "TMP FILE: " << fileinfo_file;
        QFile file( fileinfo_file );
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QTextStream stream( &file );
            stream << fileinfo_text << Qt::endl;
        }

        // NOW ADD / SAVE OUR FILE
        QProcess addfile_process;
        QStringList addfile_args;
        addfile_args << "ADDFILE" << prodosImageFile << prodosVolumeName << saveFile;   // our tmp file

        addfile_process.start(cadiusPath,addfile_args);
        addfile_process.waitForFinished();  // BLOCKS!!!
        QString addfile_output = QString(addfile_process.readAllStandardOutput());

        // IF FILE ALREADY EXISTS, ASK IF THEY WANT TO OVERWRITE
        if (addfile_output.contains("A file already exist with the same name")) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "File exists in image", "File exists, Replace?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {

                // ../tools/Cadius DELETEFILE    <[2mg|hdv|po]_image_path>   <prodos_file_path>
                // YES - DELETE
                QString deleteFile = QString("%1%2").arg(prodosVolumeName,prodosFileName);
                // qDebug() << "DELETEFILE : " <<deleteFile;
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

                addfile2_process.start(cadiusPath,addfile2_args);
                addfile2_process.waitForFinished();  // BLOCKS!!!
                QString addfile2_output = QString(addfile2_process.readAllStandardOutput());

                // IF FILE ALREADY EXISTS, ASK IF THEY WANT TO OVERWRITE
                if (addfile2_output.contains("Error :")) {
                    ui->plainTextEdit_lastCmd->document()->setPlainText("Save failed.  Couldn't delete/overwrite file?  I really am not sure what went wrong.  Try saving the pictures to your computer and transferring them to ProDOS with another program.");
                    return;
                }
            } else {
                if (ui->label_preview->pixmap().isNull()) {
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

void MainWindow::on_comboBox_dithering_currentIndexChanged(int /*unused*/)
{
    updateNeeded = 1;
}

void MainWindow::on_comboBox_previewPalette_currentIndexChanged(int /*unused*/)
{
    updateNeeded = 1;
}

// SWITCHING TABS SWITCHES ENGINES - REFRESH THE PREVIEW FOR THE NEW ONE
void MainWindow::on_tabWidget_engine_currentChanged(int /*unused*/)
{
    updateNeeded = 1;
}

void MainWindow::on_comboBox_shrTarget_currentIndexChanged(int /*unused*/)
{
    updateShrControls();
    updateNeeded = 1;
}


// DIM THE SCB MODE CONTROL FOR TARGETS THAT REJECT --scb-mode
// (single-palette targets and Brooks 3200, which has no SCBs at all)
void MainWindow::updateShrControls()
{
    const ModeDescriptor *mode = findMode(ui->comboBox_shrTarget->currentText());
    bool hasScbModes = mode && mode->shrHasScbModes;
    ui->label_shrScbMode->setEnabled(hasScbModes);
    ui->comboBox_shrScbMode->setEnabled(hasScbModes);
}

void MainWindow::on_comboBox_shrDither_currentIndexChanged(int /*unused*/)
{
    updateNeeded = 1;
}

void MainWindow::on_horizontalSlider_shrDitherStrength_valueChanged(int value)
{
    if (value==0){
        ui->label_shrDitherStrengthValue->setText("Off");
    } else {
        ui->label_shrDitherStrengthValue->setText(QString::number(value));
    }
    updateNeeded = 1;
}

void MainWindow::on_checkBox_shrSerpentine_stateChanged(int /*unused*/)
{
    updateNeeded = 1;
}

void MainWindow::on_comboBox_shrFit_currentIndexChanged(int /*unused*/)
{
    updateNeeded = 1;
}

void MainWindow::on_comboBox_shrAspect_currentIndexChanged(int /*unused*/)
{
    updateNeeded = 1;
}

void MainWindow::on_comboBox_shrScbMode_currentIndexChanged(int /*unused*/)
{
    updateNeeded = 1;
}

void MainWindow::on_comboBox_shrFormat_currentIndexChanged(int /*unused*/)
{
    updateNeeded = 1;
}

void MainWindow::on_pushButton_savePreview_clicked()
{
    if (!check_canSave()) return;
    QString a2filename;

    QString filters = QString("All Images (*.BMP *.PNG);;Bitmap (*.BMP);;PNG (*.PNG);;All files (*.*)");
    QString defaultFilter = "PNG (*.PNG)";
    QString suffix = ".PNG";

    // PROMPT FOR SAVE FILENAME AND COPY (HOPEFULLY) TO SAVE FILENAME
    QString saveFile = QFileDialog::getSaveFileName(nullptr, "Save file", QDir::currentPath(), filters, &defaultFilter);
    if (QFile::exists(saveFile)) {
        QFile::remove(saveFile);
    }

    // TRY TO SET CORRECT TYPE FOR SAVE, DEFAULT TO PNG
    QFileInfo fi(saveFile);
    QString ext = fi.completeSuffix().toUpper();
    if (ext == "BMP") {
        ui->label_preview->pixmap().save(saveFile,"BMP", 0);
    } else {
        ui->label_preview->pixmap().save(saveFile,"PNG", 0);
    }
}
