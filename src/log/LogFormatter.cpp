#include "LogFormatter.h"

#include "LogEvent.h"

namespace mytinywebserver {

class MessageFormatItem : public LogFormatter::FormatItem {
   public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
   public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << LogLevel::toString(event->getLevel());
    }
};



class NameFormatItem : public LogFormatter::FormatItem {
   public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getLoggername();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
   public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getThreadId();
    }
};

class DateFormatItem : public LogFormatter::FormatItem {
   public:
    DateFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        : m_format(format) {
        if (m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }

   private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
   public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
   public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
   public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << "\n";
    }
};

class TabFormatItem : public LogFormatter::FormatItem {
   public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << " ";
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
   public:
    StringFormatItem(const std::string& str) : m_string(str) {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << m_string;
    }

   private:
    std::string m_string;
};

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {
    init();
}

std::string LogFormatter::format(std::shared_ptr<LogEvent> event) {
    std::stringstream ss;
    for (auto& i : m_items) {
        i->format(ss, event);
    }
    return ss.str();
}

// %xxx %xxx{xxx} %%
// 默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T[%p]%T[%c]%T%f:%l%T%m%n"
void LogFormatter::init() {
    // str(d) format type
    std::vector<std::tuple<std::string, std::string, int> > vec;
    std::string nstr;  //普通字符串 不是%d这种的 类似于printf("i love %d", 1); i
                       // love 是nstr，d是str fmt是{}中的东西
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        // 没遇到%，记录普通字符
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }
        // 遇到% 就判断下一个是不是 % ，是就 当前str为%，然后继续往后处理
        size_t n = i + 1;
        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        int fms_status = 0;  //是否遇到左括号 这个格式xxx{xxx}
        std::string str;
        std::string fmt;
        size_t fmt_begin = 0;

        // 看当前%之后的格式，直到%后面xxx
        // xxx{xxx}这个格式被解析出来，则跳出来让外循环处理下一个%后面的格式
        while (n < m_pattern.size()) {
            // 如果当前格式是不是中断点，例如，空格符 中断就退出去
            if (fms_status == 0 && m_pattern[n] != '{' && m_pattern[n] != '}' &&
                !isalpha(m_pattern[n])) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            // 如果格式还在继续，这时候 有可能等于左大括号，右大括号
            // 遇到左括号，str确定
            if (fms_status == 0 && m_pattern[n] == '{') {
                str = m_pattern.substr(i + 1, n - i - 1);
                fms_status = 1;  //要去匹配有括号
                fmt_begin = n;   // 记录fmt的起始位置
                ++n;             // fix
                continue;        // fix
            }
            // 遇到右括号，fmt确定，然后退出
            if (fms_status == 1 && m_pattern[n] == '}') {
                fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                fms_status = 0;
                ++n;  // fix
                break;
            }
            // 都没有遇到
            ++n;
            // 没遇到中断点，也没遇到左括号，%就结束了，则记录下str //fix
            if (n == m_pattern.size()) {
                if (str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }
        // 要不就是当前%后面的格式 没有左括号 要不是就是已经匹配到了右括号
        if (fms_status == 0) {
            // 普通字符记得放进去
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;  // fix
        } else if (fms_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - "
                      << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }
    // 解析完了，普通字符可能在最后，没放进去，现在放进去
    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    //根据str得到FormatItemptr
    static std::map<std::string, std::function<std::shared_ptr<FormatItem>(
                                     const std::string& str)> >
        s_format_items = {
#define XX(str, C)                                                            \
    {                                                                         \
#str,                                                                 \
            [](const std::string&                                             \
                   str1) { return std::shared_ptr<FormatItem>(new C(str1)); } \
    }
            // 默认格式 "%d{%Y-%m-%d
            // %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
            XX(m, MessageFormatItem),     // m:消息
            XX(p, LevelFormatItem),       // p:日志级别
            XX(c, NameFormatItem),        // c:日志名称
            XX(t, ThreadIdFormatItem),    // t:线程id
            XX(n, NewLineFormatItem),     // n:换行
            XX(d, DateFormatItem),        // d:时间
            XX(f, FilenameFormatItem),    // f:文件名
            XX(l, LineFormatItem),        // l:行号
            XX(T, TabFormatItem),         // T:Tab
#undef XX
        };

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {  // 如果type = 0，没有fmt，就是普通字符
            m_items.push_back(
                FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(
                    "<<error_format % " + std::get<0>(i) + ">")));
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
    }
}

}  // namespace mytinywebserver