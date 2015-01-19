#ifndef CONFIG_H
#define CONFIG_H

struct Config {
    using String = std::wstring;

    struct {
        int     width;
        int     height;
        int     xPos;
        int     yPos;
        int     lineSpacing;
        struct {
            int foreground;
            int background;
        } color;
        String  format;
    } text;

    struct {
        String  name;
        int     size;
        int     italic;
        int     bold;
        int     langId;
        int     quality;
    } font;

    struct {
        bool        isXpStyle;
    } explore;
};

Config  loadConfig();
void    saveConfig(const Config& config);

#endif
