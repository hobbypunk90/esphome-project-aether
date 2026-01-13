#include "esphome.h"

using namespace esphome;
using namespace esphome::display;
using namespace esphome::font;

static const int HEADER_HEIGHT = 24;

struct TextMetrics {
  int width;
  int height;
  int baseline;
  int x_offset;
};

template <typename P> class Container {
protected:
  P &parent_;
  int x_offset_;
  int y_offset_;
  int width_;
  int height_;

public:
  // Konstruktor: Wo startet der Bereich und wie groß ist er?
  Container(P &parent, int x, int y, int width, int height)
      : parent_(parent), x_offset_(x), y_offset_(y), width_(width),
        height_(height) {
    ESP_LOGD("layout", "Container: (%dx%d)@%d,%d", width, height, x, y);
  }

  // --- Wrapper, die den Offset addieren ---

  template <typename... Args> void print(int x, int y, Args... args) {
    // Wir rufen den Parent auf, schieben aber x und y
    parent_.print(x_offset_ + x, y_offset_ + y, args...);
  }

  template <typename... Args> void printf(int x, int y, Args... args) {
    parent_.printf(x_offset_ + x, y_offset_ + y, args...);
  }

  template <typename... Args> void image(int x, int y, Args... args) {
    parent_.image(x_offset_ + x, y_offset_ + y, args...);
  }

  template <typename... Args>
  void line(int x1, int y1, int x2, int y2, Args... args) {
    parent_.line(x_offset_ + x1, y_offset_ + y1, x_offset_ + x2, y_offset_ + y2,
                 args...);
  }

  template <typename... Args>
  void rectangle(int x, int y, int w, int h, Args... args) {
    parent_.rectangle(x_offset_ + x, y_offset_ + y, w, h, args...);
  }

  template <typename... Args>
  void filled_rectangle(int x, int y, int w, int h, Args... args) {
    parent_.filled_rectangle(x_offset_ + x, y_offset_ + y, w, h, args...);
  }

  // --- Helpers ---

  // Gibt die Breite DES BEREICHS zurück (nicht des Displays!)
  // Das ist Gold wert für Zentrierung innerhalb der Box
  P &get_parent() { return parent_; }
  int get_width() { return width_; }
  int get_height() { return height_; }
  int get_x() { return x_offset_; }
  int get_y() { return y_offset_; }

  int get_text_width(Font *font, const char *text) {
    int w, x_off, baseline, h;
    font->measure(text, &w, &x_off, &baseline, &h);

    return w;
  }

  int get_cap_height(Font *font) { return font->get_capheight(); }

  TextMetrics get_text_metrics(Font *font, const char *text) {
    int w, x_off, baseline, h;
    // Wir rufen measure nur EINMAL auf
    font->measure(text, &w, &x_off, &baseline, &h);

    // Und geben das Paket zurück
    return {w, h, baseline, x_off};
  }

  void start_clipping(int x, int y, int width, int height) {
    parent_.start_clipping(x + x_offset_, y + y_offset_, width, height);
  }

  void end_clipping() { parent_.end_clipping(); }
};

template <typename T> class Page : public Container<T> {
public:
  Page(T &it)
      : Container<T>(it, 0, HEADER_HEIGHT, it.get_width(),
                     it.get_height() - HEADER_HEIGHT) {
    header(it);
  }

private:
  void header(T &it) {
    it.strftime(4, HEADER_HEIGHT - 3, &id(header_font),
                TextAlign::BASELINE_LEFT, "%d.%m %H:%M", id(time1).now());

    // icons
    const auto wifi_pos = 0;
    header_icons(it, it.get_width() - 4, HEADER_HEIGHT - 3,
                 TextAlign::BASELINE_RIGHT);

    it.line(0, HEADER_HEIGHT - 1, it.get_width(), HEADER_HEIGHT - 1);
  }

  void header_icons(T &it, int x, int y, TextAlign align) {
    // WiFi Icon
    const auto wifi_percent = id(wifi_signal_percent).state;
    const char *icon_wifi = "\U000F0928";
    if (wifi_percent <= 0)
      icon_wifi = "\U000F092E";
    else if (wifi_percent <= 20)
      icon_wifi = "\U000F092F";
    else if (wifi_percent <= 40)
      icon_wifi = "\U000F091F";
    else if (wifi_percent <= 60)
      icon_wifi = "\U000F0922";
    else if (wifi_percent <= 80)
      icon_wifi = "\U000F0925";

    // API Icon
    const char *icon_api = "\U000F1511";
    if (id(api_default).is_connected())
      icon_api = "\U000F0003";

    it.print(x, y + 2, &id(header_font), align,
             str_sprintf("%s%s", icon_api, icon_wifi).c_str());
  }
};

template <typename T> class BootPage : public Page<T> {
public:
  BootPage(T &it) : Page<T>(it) {
    this->print(this->get_width() / 2, this->get_height() / 2,
                &id(page_filling_icons), TextAlign::CENTER, "\U000F07D0");
  }
};