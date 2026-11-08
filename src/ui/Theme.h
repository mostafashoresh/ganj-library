#pragma once

class QApplication;

namespace Theme {
void loadFonts();                // loads any .ttf/.otf placed in <app folder>/fonts (e.g. Vazirmatn)
void apply(QApplication& app);   // Fusion style + stylesheet + RTL font
}
