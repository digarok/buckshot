#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QTemporaryDir>

#include "modedescriptor.h"

namespace Ui {
class MainWindow;
}

/*
 *  280x192
 *  ~306 x 210
 */

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:


    void on_pushButton_sourceFilename_clicked();

    void on_horizontalSlider_crossHatch_valueChanged(int value);
    void on_horizontalSlider_colorBleed_valueChanged(int value);
    void on_comboBox_outputFormat_currentIndexChanged(int);
    void on_comboBox_inputResolution_currentIndexChanged(int index);

    void on_checkBox_livePreview_stateChanged(int arg1);
    void previewTimerTimeout();

    void on_actionWhat_is_this_triggered();

    void on_pushButton_preview_clicked();
    void on_pushButton_saveImage_clicked();
    void on_pushButton_saveToProdos_clicked();

    void on_comboBox_dithering_currentIndexChanged(int index);
    void on_comboBox_previewPalette_currentIndexChanged(int index);

    void on_pushButton_savePreview_clicked();

    void on_tabWidget_engine_currentChanged(int index);
    void on_comboBox_shrTarget_currentIndexChanged(int index);
    void on_comboBox_shrDither_currentIndexChanged(int index);
    void on_horizontalSlider_shrDitherStrength_valueChanged(int value);
    void on_checkBox_shrSerpentine_stateChanged(int arg1);
    void on_comboBox_shrFit_currentIndexChanged(int index);
    void on_comboBox_shrAspect_currentIndexChanged(int index);
    void on_comboBox_shrScbMode_currentIndexChanged(int index);
    void on_comboBox_shrFormat_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    const ModeDescriptor &currentMode() const;
    void runB2dConversion(const ModeDescriptor &mode);
    void runShrConversion(const ModeDescriptor &mode);
    void showPreviewAndSource(const QPixmap &previewPix);
    void updateInputSize();
    void livePreview();
    void updateDisplayModes();
    void updateShrControls();
    bool check_canSave();
    bool check_canPreview();

    QPixmap sourcePixmap;   // original full-res source; the labels only show scaled copies
    int inputWidth;
    int inputHeight;
    bool updateNeeded;
    QTimer *previewTimer;

    QTemporaryDir *tmpDir;
    QString tmpDirPath;
    static const QString imageName;     //"saved"
    static const QString version;       //"0.00"
    static const QString programName;   //"buckshot"

    QString inputImgPath;
    QString previewImgPath;
    QString outputImgPath;

    QString shrPreviewImgPath;
    QString shrOutputPath;       // set per conversion; honors a --format brooks override
    QString shrJsonFileType;     // ProDOS type/aux reported by image2shr --json
    QString shrJsonAuxType;

    // What the preview pixmap was generated with, so saves refuse stale results.
    Engine lastPreviewEngine = Engine::B2D;
    QString lastPreviewModeName;
};

#endif // MAINWINDOW_H
