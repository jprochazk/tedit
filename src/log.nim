import terminal

type
    LogLevel* = enum
        lvlDebug = "DEBUG"
        lvlInfo = "INFO"
        lvlNotice = "NOTICE"
        lvlWarn = "WARN"
        lvlError = "ERROR"
        lvlFatal = "FATAL"
# Logging level, can be changed
var logLevel* = lvlDebug

proc log*(lvl: LogLevel, msg: string) =
    ## Logs message with specified log level to the stdout
    # Only print messages with specified log level or higher
    if lvl < logLevel: return
    case lvl
    of lvlDebug: stdout.styledWriteLine("[", fgWhite, "DEBUG", "]: ", msg)
    of lvlInfo: stdout.styledWriteLine("[", fgGreen, "INFO", "]: ", msg)
    of lvlNotice: stdout.styledWriteLine("[", fgCyan, "NOTICE", "]: ", msg)
    of lvlWarn: stdout.styledWriteLine("[", fgYellow, "WARN", "]: ", msg)
    of lvlError: stdout.styledWriteLine("[", fgRed, "ERROR", "]: ", msg)
    of lvlFatal: stdout.styledWriteLine("[", fgRed, "FATAL", "]: ", msg)
