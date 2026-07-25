#include "modedescriptor.h"

// b2d invocation: <input.bmp> <format token> [N] [Z<n>] [R<n>] [D<n>] [V<n>] <extra args>
// The V flag must always be present so b2d emits the preview BMP.
static QStringList buildB2dArgs(const ModeDescriptor &mode, const ConversionParams &params)
{
    QStringList args;
    args << params.inputPath;
    args << mode.b2dToken;
    if (mode.b2dNeedsN) {
        args << "N";
    }
    if (params.crossHatch > 0) {
        args << QString("Z%1").arg(params.crossHatch);
    }
    if (params.colorBleed > 0) {
        args << QString("R%1").arg(params.colorBleed);
    }
    if (params.ditherIndex > 0) {
        args << QString("D%1").arg(params.ditherIndex);
    }
    if (params.paletteIndex > -1) {
        args << QString("V%1").arg(params.paletteIndex);
    }
    args << params.extraArgsText;
    return args;
}

// image2shr invocation: convert [flags] <input>
// All flags precede the positional input path (Go flag parsing).
// --json is always requested so results (ProDOS type/aux, warnings) can be read.
static QStringList buildShrArgs(const ModeDescriptor &mode, const ConversionParams &params)
{
    QStringList args;
    args << "convert";
    args << "-t" << mode.shrTarget;
    args << "--dither" << params.shrDither;
    args << "--dither-strength" << QString::number(params.shrDitherStrength);
    if (params.shrSerpentine) {
        args << "--serpentine";
    }
    args << "--fit" << params.shrFit;
    args << "--aspect" << params.shrAspect;
    args << "--scb-mode" << params.shrScbMode;
    args << "--format" << params.shrFormat;
    args << "--preview-png" << params.previewPath;
    args << "--json";
    args << "-o" << params.outputPath;
    args += params.extraArgsText.split(' ', Qt::SkipEmptyParts);
    args << params.inputPath;
    return args;
}

//---- input resolutions (comboBox_inputResolution indices)
// 0  "40 x 48   - Full Scale LGR (LGR ONLY)"
// 1  "80 x 48   - Full Scale DLGR (DLGR ONLY)"
// 2  "140 x 192 - Full Scale (HGR & DHGR)"
// 3  "280 x 192 - Double Width Scale (HGR & DHGR)"
// 4  "320 x 200 - Classic Size"
// 5  "560 x 384 - Quadruple Width, Double Height Scale"
// 6  "640 x 400 - Classic Size"
// 7  "640 x 480 - Classic Size"
const QVector<ModeDescriptor> &allModes()
{
    static const QVector<ModeDescriptor> modes = {
        { "LR", Engine::B2D,
          "L", true, {2, 3}, 0,
          QString(),
          "SAVED.SLO", "LR (*.SLO)", "06", "0400",
          buildB2dArgs },
        { "DLR", Engine::B2D,
          "DL", true, {2, 3}, 1,
          QString(),
          "SAVED.DLO", "DLR (*.DLO)", "06", "0400",
          buildB2dArgs },
        { "HGR", Engine::B2D,
          "H", false, {0, 1}, 2,
          QString(),
          // b2d names HGR output <NAME><options>.BIN; "H" yields options "CH"
          "SAVEDCH.BIN", "HGR (*.BIN)", "06", "2000",
          buildB2dArgs },
        { "HGR Nearest Pixel Group", Engine::B2D,
          "hgr2", false, {0, 1}, 2,   // tohgr-style alternate HGR conversion
          QString(),
          "SAVEDCA.BIN", "HGR (*.BIN)", "06", "2000",
          buildB2dArgs },
        { "DHGR", Engine::B2D,
          "D", false, {0, 1}, 3,
          QString(),
          "SAVED.A2FC", "DHGR (*.A2FC)", "06", "2000",
          buildB2dArgs },
        { "HGR MONO", Engine::B2D,
          // b2d picks HGR mono output from the 280 x 192 input size
          "mono", false, {0, 1, 2, 4, 5, 6, 7}, 3,
          QString(),
          "SAVEDM.BIN", "HGR (*.BIN)", "06", "2000",
          buildB2dArgs },
        { "DHGR MONO", Engine::B2D,
          // b2d picks DHGR mono output from the 560 x 384 input size
          "mono", false, {0, 1, 2, 3, 4, 6, 7}, 5,
          QString(),
          "SAVED.A2FM", "DHGR MONO (*.A2FM)", "06", "2000",
          buildB2dArgs },
        // image2shr scales the source itself, so the resolution fields are unused.
        // outputFileName/type/aux reflect --format auto; a brooks override at
        // runtime switches to SAVED.3200 and the --json-reported type/aux.
        { "SHR 320 Grey 16", Engine::Image2SHR,
          QString(), false, {}, 0,
          "shr320-grey16",
          "SAVED.SHR", "SHR (*.SHR)", "C1", "0000",
          buildShrArgs },
        { "SHR 320 Color 16", Engine::Image2SHR,
          QString(), false, {}, 0,
          "shr320-color16",
          "SAVED.SHR", "SHR (*.SHR)", "C1", "0000",
          buildShrArgs },
        { "SHR 320 Color 256", Engine::Image2SHR,
          QString(), false, {}, 0,
          "shr320-color256",
          "SAVED.SHR", "SHR (*.SHR)", "C1", "0000",
          buildShrArgs },
        { "SHR 320 Color 3200", Engine::Image2SHR,
          QString(), false, {}, 0,
          "shr320-color3200",
          "SAVED.3200", "Brooks 3200 (*.3200)", "C1", "0002",
          buildShrArgs },
    };
    return modes;
}

const ModeDescriptor *findMode(const QString &name)
{
    for (const ModeDescriptor &mode : allModes()) {
        if (mode.name == name) {
            return &mode;
        }
    }
    return nullptr;
}

QStringList modeNamesForEngine(Engine engine)
{
    QStringList names;
    for (const ModeDescriptor &mode : allModes()) {
        if (mode.engine == engine) {
            names << mode.name;
        }
    }
    return names;
}
