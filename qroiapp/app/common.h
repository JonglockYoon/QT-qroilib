#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <QTime>
#include <QMutex>
#include <QElapsedTimer>
#include <ctype.h>
#include <opencv2/core/core.hpp>


#ifndef PI
#define PI 3.141592653589793f
#endif
//두점 사이의 각도 구하기
#define CalculDegree(from, to)  -((double)atan2(to.y - from.y, to.x - from.x) * 180.0f / PI)
#define RADIAN(angle) angle *  PI /180


typedef struct stat Stat;

using namespace std;


typedef struct {
    int point;
    double x;
    double y;
    double z;
    double dx; // 얼라인 이동 거리.
    double dy;
} SCREWITEM;

typedef enum
{
    eAUTO_RUN=1,
    //eINLINE_PALLET,
    //eINLINE_TRAY,
    //eINLINE_BYPASS,
    eDRY_RUN
} eAutoMode_Button;

typedef enum
{
    eAXIS_X = 0,
    eAXIS_MAX
} eAxisInfo;

typedef enum
{
    eJOG_SPEED_LOW=0,
    eJOG_SPEED_MID,
    eJOG_SPEED_HIGH,
    eJOG_SPEED_MAX
} eJogSpeedInfo;

typedef enum enumStateOPStatus
{
    INIT_STATUS = 0,
    ERROR_STOP,
    STEP_STOP,
    STEP_READY,
    START_RUN,
    RUN
}EOPStatus;

typedef enum
{
    JOG_P_X=0,
    JOG_N_X,
    JOG_P_Y,
    JOG_N_Y,
    JOG_P_Z,
    JOG_N_Z,
    JOG_P_W,
    JOG_N_W,
    JOG_MAX
} eJogDirInfo;

// 메모리 관련 편의 함수
#ifndef SAFE_DELETE
#define SAFE_DELETE(P)			{ if(P) { delete (P); (P) = NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(P)	{ if(P) { delete[] (P); (P) = NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(P)			{ if(P) { (P)->Release(); (P) = NULL; } }
#endif


#define BUTTON_UPDATE_TEXT(BTN, S) \
{ \
    QString s = (S); \
    if((BTN)->text() != s) \
    { \
        (BTN)->setText(s); \
    } \
}

#include <QProgressDialog>

class ProgressDialog : public QProgressDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = Q_NULLPTR)
        : QProgressDialog(parent)
    {
        setWindowTitle(tr("Motor Origin Executing..."));
        //setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        setLabelText(tr("...is executing the Motor Origin...\nWait a moment..."));
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

//#ifndef SetError
//#define SetError(CODE)	__SetError(CODE, __FILE__, __LINE__)
//#endif

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

extern int hexstr2dec(char *hex);

#endif // COMMON_H
