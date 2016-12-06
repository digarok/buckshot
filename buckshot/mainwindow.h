#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTemporaryDir>

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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:


    void on_pushButton_sourceFilename_clicked();

    void on_pushButton_preview_clicked();

    void on_horizontalSlider_crossHatch_valueChanged(int value);

    void on_horizontalSlider_colorBleed_valueChanged(int value);

    void on_comboBox_outputFormat_currentIndexChanged(int index);

    void on_comboBox_inputResolution_currentIndexChanged(int index);

    void on_checkBox_livePreview_stateChanged(int arg1);
    void previewTimerTimeout();
    void on_pushButton_saveImage_pressed();

    void on_actionWhat_is_this_triggered();

private:
    Ui::MainWindow *ui;
    void updateInputSize();
    void livePreview();
    void updateDisplayModes();
    int inputWidth;
    int inputHeight;
    bool updateNeeded;
    QTimer *previewTimer;

    QTemporaryDir *tmpDir;
    QString tmpDirPath;
    static const QString imageName; //"saved"
    QString inputImgPath;
    QString previewImgPath;
    QString outputImgPath;
};

#endif // MAINWINDOW_H
