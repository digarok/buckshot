#ifndef MODEDESCRIPTOR_H
#define MODEDESCRIPTOR_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>

enum class Engine { B2D, Image2SHR };

// Snapshot of the UI state needed to build a converter command line,
// so the descriptor table stays free of widget access.
struct ConversionParams {
    QString inputPath;
    QString outputPath;      // image2shr -o target; b2d derives its own names
    QString previewPath;     // image2shr --preview-png; b2d always writes <name>_Preview.bmp
    QString extraArgsText;   // raw "Additional arguments" line

    // b2d knobs
    int crossHatch = 0;
    int colorBleed = 0;
    int ditherIndex = 0;
    int paletteIndex = 0;

    // image2shr knobs (CLI tokens)
    QString shrDither;
    double shrDitherStrength = 1.0;
    bool shrSerpentine = false;
    QString shrFit;
    QString shrAspect;
    QString shrScbMode;
    QString shrFormat;
};

struct ModeDescriptor {
    QString name;            // combobox display text
    Engine engine;

    // b2d: positional format token and input-resolution rules
    QString b2dToken;                // "L", "DL", "H", "hgr2", "D", "mono"
    bool b2dNeedsN = false;          // LR/DLR append "N"
    QList<int> disabledResolutions;  // indices into comboBox_inputResolution
    int defaultResolution = 2;

    // image2shr: conversion target
    QString shrTarget;               // "shr320-color16", ...

    // output/save facts
    QString outputFileName;          // file the converter leaves in the temp dir
    QString saveFilter;              // default file-dialog filter
    QString prodosFileType;          // "06" (BIN) or "C1" (PIC)
    QString prodosAuxType;

    QStringList (*buildArgs)(const ModeDescriptor &mode, const ConversionParams &params);
};

const QVector<ModeDescriptor> &allModes();
const ModeDescriptor *findMode(const QString &name);
QStringList modeNamesForEngine(Engine engine);

#endif // MODEDESCRIPTOR_H
