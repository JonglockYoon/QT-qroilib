#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <QTime>
#include <QMutex>
#include <QElapsedTimer>
#include <ctype.h>
#include <opencv2/core/core.hpp>

using namespace std;
//using namespace cv;

#ifndef PI
#define PI 3.141592653589793f
#endif
//두점 사이의 각도 구하기
#define CalculDegree(from, to)  -((double)atan2(to.y - from.y, to.x - from.x) * 180.0f / PI)
#define RADIAN(angle) angle *  PI /180


typedef struct stat Stat;

#include <QProgressDialog>

class ProgressDialog : public QProgressDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = Q_NULLPTR)
        : QProgressDialog(parent)
    {
        setWindowTitle(tr("Wait"));
        setLabelText(tr("...is executing...\nWait a moment..."));
        setMinimum(0);
        setValue(0);
        setMinimumDuration(0);
    }

   void setProgressVal(qint64 nVal, qint64 nMax)
   {
       setMaximum(nMax);
       setValue(nVal);
   }
};

extern void Tabs2Spaces(char *pBuff);
extern string string_format(const string fmt, ...);
extern void GetPrivateProfileString(string sectionName, char *pTitle, char *Default, char *szData, int nLen, string pathname);
extern void WritePrivateProfileString(string sectionName, char *pTitle, char *szData, string pathname);
extern int GetPrivateProfileInt(string sectionName, char *pTitle, int Default, string pathname);
extern char* format(char* dst, const char* src);
extern string format(const string& str);

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

extern QString toUniString(char* str);
extern QString rstrip(const QString& str);

extern void qimage_to_mat(const QImage* image, cv::OutputArray out);
extern void mat_to_qimage(cv::InputArray image, QImage& out);

#endif // COMMON_H
