typedef struct Logger
{
    void (*dev)(const char* fmt, ...);
    void (*debug)(const char* fmt, ...);
    void (*info)(const char* fmt, ...);
    void (*warning)(const char* fmt, ...);
    void (*error)(const char* fmt, ...);
    void (*critical)(const char* fmt, ...);
};

void logger_init(struct Logger* logger);

void _dev(const char* fmt, ...);
void _debug(const char* fmt, ...);
void _info(const char* fmt, ...);
void _warning(const char* fmt, ...);
void _error(const char* fmt, ...);
void _critical(const char* fmt, ...);