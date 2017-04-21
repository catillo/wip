#define SERIAL_BAUD_RATE              230400
#include <memory>
#include <string>

#define LWIP_INTERNAL
#include <ESP8266WiFi.h>

extern "C"
{
#include "include/wl_definitions.h"
#include "osapi.h"
#include "ets_sys.h"
void esp_yield();
}

#include "lwip/opt.h"
#include "lwip/ip_addr.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"

#if 0
#include "debug.h"
#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include "WiFiServer.h"
#include "lwip/opt.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "include/ClientContext.h"
#include "c_types.h"
#endif

#define TX_IO   1
#define RX_IO   3
#define D8      15
#define D7      13
#define D6      12
#define D5      14
#define D4      2
#define D3      0
#define D2      4
#define D1      5
#define D0      16
#define CLK_IO        6
#define SPI_MIS0_IO   7
#define SPI_CS0_IO    11
#define SPI_MOS1_IO   8
#define SPIHD_IO      9
#define SPIWP_IO      10

/*******************************************************************************************************************************/
class Element {
public:
    Element *_next;
    Element();
    virtual ~Element();
};

class ListIterator;
class ListElem : public Element {
private:
    Element *_head;
    friend class ListIterator;
public:
    ListElem();
    ~ListElem();
    bool add(std::unique_ptr<Element> e);
    std::unique_ptr<ListIterator> iterator();
};

class ListIterator {
private:
    ListElem *_owner;
    unsigned int _index;
public:
    ListIterator(ListElem *owner);
    Element* next();
    bool hasNext();

    ListIterator();
    ~ListIterator();
};

class TimerFunction : public Element {
private:
    unsigned long _interval;
    unsigned long _timeToNextRun;
public:
    TimerFunction();
    virtual ~TimerFunction();
    void update(unsigned long elapsed_millis);
    virtual void run(unsigned long elapsed_millis) = 0;
    void setInterval(unsigned long interval, unsigned long elapsed_millis);
};

class TimerSys {
private:
public:
    static ListElem _functions;
    static bool register_func(std::unique_ptr<TimerFunction> tf, unsigned long interval, unsigned long elapsed_millis);
    static void update(unsigned long elapsed_millis);
};

ListElem TimerSys::_functions;

Element::Element() : _next(0) { }
Element::~Element() { }

/*******************************************************************************************************************************/
ListElem::ListElem() : _head(0) { }

ListElem::~ListElem() {
    while (_head != NULL) {
        Element *tmp = _head;
        _head = _head->_next;

        delete tmp;
    }
}

bool ListElem::add(std::unique_ptr<Element> e) {
    if (e.get() == NULL) {
        return false;
    }

    if (_head == NULL) {
        _head = e.release();
        _head->_next = NULL;
    } else {
        Element *runner = _head;
        while (runner->_next != NULL) {
            runner = runner->_next;
        }

        runner->_next = e.release();
        runner->_next->_next = NULL;
    }

    return true;
}

std::unique_ptr<ListIterator> ListElem::iterator() {
    std::unique_ptr<ListIterator> r(new ListIterator(this));

    return std::move(r);
}

/*******************************************************************************************************************************/
ListIterator::ListIterator(ListElem *owner) : _owner(owner), _index(0) { }
ListIterator::~ListIterator() { }

Element* ListIterator::next() {
    Element *runner = _owner->_head;
    unsigned int i = 0;
    while (runner != NULL) {
        if (i == _index) {
            _index++;
            return runner;
        }

        i++;
        runner = runner->_next;
    }

    return NULL;
}

bool ListIterator::hasNext() {
    Element *runner = _owner->_head;
    unsigned int i = 0;
    while (runner != NULL) {
        if (i == _index) {
            return true;
        }

        i++;
        runner = runner->_next;
    }

    return false;
}

/*******************************************************************************************************************************/

void TimerFunction::setInterval(unsigned long interval, unsigned long elapsed_millis) {
    _interval = interval;
    _timeToNextRun = elapsed_millis + _interval;
}

TimerFunction::TimerFunction() : _interval(0), _timeToNextRun(0) { }
TimerFunction::~TimerFunction() { }

void TimerFunction::update(unsigned long elapsed_millis) {
    if (_interval == 0) {
        run(elapsed_millis);
    } else if (elapsed_millis >= _timeToNextRun) {
        run(elapsed_millis);
        _timeToNextRun += _interval;
    }
}

/*******************************************************************************************************************************/

bool TimerSys::register_func(std::unique_ptr<TimerFunction> tf, unsigned long interval, unsigned long elapsed_millis) {
    tf->setInterval(interval, elapsed_millis);
    std::unique_ptr<Element> t(tf.release());
    return TimerSys::_functions.add(std::move(t));
}

void TimerSys::update(unsigned long elapsed_millis) {
    std::unique_ptr<ListIterator> i = TimerSys::_functions.iterator();

    while (i->hasNext()) {
        TimerFunction *tf = static_cast<TimerFunction *>(i->next());
        tf->update(elapsed_millis);
    }
}

/*******************************************************************************************************************************/

class InputPinReader : public Element {
private:
    int _pin;
public:
    int _high;
    InputPinReader(int pin);
    virtual ~InputPinReader();
    void run();
};

InputPinReader::InputPinReader(int pin) : _pin(pin), _high(false) {
    pinMode(_pin, INPUT);
}

InputPinReader::~InputPinReader() { }

void InputPinReader::run() {
    if (digitalRead(_pin) == HIGH) {
        _high = true;
    } else {
        _high = false;
    }
}

/*******************************************************************************************************************************/
#define DEVSTAT_LED 0x01

class StatusLed : public TimerFunction {
public:
    typedef enum {
        LED_STATE_CONNECTED = 0
        , LED_STATE_ERROR
        , LED_STATE_SEARCHING
        , LED_STATE_AP_MODE
        , LED_STATE_INVALID
    } LED_STATE;

private:
    typedef struct {
        LED_STATE _state;
        unsigned int _pattern;
    } StatusLedPatternType;

    static StatusLedPatternType _patterns[];
    LED_STATE _currState;
    unsigned int _currPattern;
    unsigned int _patternBitRunner;
    unsigned int* findPattern(const LED_STATE state);
public:
    LED_STATE _newState;

    StatusLed();
    ~StatusLed();
    void run(unsigned long elapsed_millis);
};

StatusLed::StatusLedPatternType StatusLed::_patterns[] = {
    { LED_STATE_CONNECTED, 0x8000 }, { LED_STATE_ERROR, 0xAAAA }, { LED_STATE_SEARCHING, 0xF0F0 }, { LED_STATE_AP_MODE, 0xA000 }, { LED_STATE_INVALID, 0xFFFF }
};

unsigned int* StatusLed::findPattern(const LED_STATE state) {
    StatusLedPatternType *runner = _patterns;
    while (runner->_state != LED_STATE_INVALID) {
        if (runner->_state == state) {
            return &runner->_pattern;
        }
        runner++;
    }
    return NULL;
}

StatusLed::StatusLed() :
   _currState(LED_STATE_SEARCHING)
   , _currPattern(*findPattern(_currState))
   , _patternBitRunner(0x8000)
   , _newState(_currState) {
    pinMode(D0, OUTPUT);
    digitalWrite(D0, LOW);

}

StatusLed::~StatusLed() {

}

void StatusLed::run(unsigned long elapsed_millis) {
    if (_patternBitRunner == 0x0000) {
        _patternBitRunner = 0x8000;

        if (_newState != _currState) {
            _currState = _newState;

            _currPattern = *findPattern(_currState);
        }
    }

    if (_currPattern & _patternBitRunner) {
        digitalWrite(D0, LOW);
    } else {
        digitalWrite(D0, HIGH);
    }

    _patternBitRunner >>= 1;
}


/*******************************************************************************************************************************/
#define MOTOR_IOCTL_STATE_INDEX_FORWARD     0
#define MOTOR_IOCTL_STATE_INDEX_REVERSE     1
#define MOTOR_IOCTL_STATE_INDEX_LEFT        2
#define MOTOR_IOCTL_STATE_INDEX_RIGHT       3
#define MOTOR_IOCTL_STATE_INDEX_STOP        4

#define MOTOR_IOCTL_STATE_MASK_INVALID  0xFF
#define MOTOR_IOCTL_STATE_MASK_1        0x8
#define MOTOR_IOCTL_STATE_MASK_2        0x4
#define MOTOR_IOCTL_STATE_MASK_3        0x2
#define MOTOR_IOCTL_STATE_MASK_4        0x1

class Motors : public TimerFunction {
private:
    typedef struct {
        unsigned char mask;
        unsigned char pin;
    } MaskAndPin;

    typedef enum {
        MOTOR_STATE_STOP = 0,
        MOTOR_STATE_MOVING,
        MOTOR_STATE_TRANSITIONING
    } MotorState;

    unsigned char _currIOCtlStateIndex;
    unsigned char _newIOCtlStateIndex;
    MotorState _motorState;
    unsigned long _transitionStartMillis;

    static unsigned char _ioctlStates[];
    static MaskAndPin maskAndPin[];
    void engage(unsigned char mask);
    void run(unsigned long elapsed_millis);
public:
    Motors();
    ~Motors();
    void forward();
    void reverse();
    void turnLeft();
    void turnRight();
    void stop();
};

unsigned char Motors::_ioctlStates[] = {
    0xA // forward
    , 0x5 // reverse
    , 0x9 // left
    , 0x6 // right
    , 0x0 // stop
    , 0xFF // terminator / invalid state
};

Motors::MaskAndPin Motors::maskAndPin[] = {
    { MOTOR_IOCTL_STATE_MASK_1, D1 }, { MOTOR_IOCTL_STATE_MASK_2, D2 }, { MOTOR_IOCTL_STATE_MASK_3, D5 }, { MOTOR_IOCTL_STATE_MASK_4, D6 }, { MOTOR_IOCTL_STATE_MASK_INVALID, 0 }
};

void Motors::forward() {
    _newIOCtlStateIndex = MOTOR_IOCTL_STATE_INDEX_FORWARD;
}

void Motors::reverse() {
    _newIOCtlStateIndex = MOTOR_IOCTL_STATE_INDEX_REVERSE;
}

void Motors::turnLeft() {
    _newIOCtlStateIndex = MOTOR_IOCTL_STATE_INDEX_LEFT;
}

void Motors::turnRight() {
    _newIOCtlStateIndex = MOTOR_IOCTL_STATE_INDEX_RIGHT;
}

void Motors::stop() {
    _newIOCtlStateIndex = MOTOR_IOCTL_STATE_INDEX_STOP;
}

void Motors::engage(unsigned char stateIndex) {
    if (stateIndex >= sizeof(_ioctlStates)) { //invalid param
        return;
    }

    MaskAndPin *runner = maskAndPin;
    while (runner->mask != MOTOR_IOCTL_STATE_MASK_INVALID) {
        digitalWrite(runner->pin, (_ioctlStates[stateIndex] & runner->mask) == 0 ? LOW : HIGH);
        runner++;
    }
}

void Motors::run(unsigned long elapsed_millis) {
#if 0
    static long int last_elapsed = 0;
    static unsigned char currIOCtlState = MOTOR_IOCTL_STATE_INDEX_STOP;
    if (last_elapsed == 0) {
        last_elapsed = elapsed_millis;
    } else {
        if (elapsed_millis - last_elapsed >= 3000) {
            last_elapsed = elapsed_millis;
            currIOCtlState++;
            if (_ioctlStates[currIOCtlState] == 0xFF) {
                currIOCtlState = MOTOR_IOCTL_STATE_INDEX_FORWARD;
            }
            _newIOCtlStateIndex = currIOCtlState;
        }
    }
#endif

    if (_motorState == MOTOR_STATE_STOP) {
        if (_newIOCtlStateIndex != _currIOCtlStateIndex) {
            _currIOCtlStateIndex = _newIOCtlStateIndex;
            engage(_currIOCtlStateIndex);
            _motorState = MOTOR_STATE_MOVING;
        }
        return;
    } else if (_motorState == MOTOR_STATE_TRANSITIONING) {
        if ((elapsed_millis - _transitionStartMillis) >= 150) {
            engage(_currIOCtlStateIndex);
            _motorState = MOTOR_STATE_MOVING;
            return;
        }

    } else if (_motorState == MOTOR_STATE_MOVING) {
        if (_newIOCtlStateIndex != _currIOCtlStateIndex) {
            _currIOCtlStateIndex = _newIOCtlStateIndex;
            engage(MOTOR_IOCTL_STATE_INDEX_STOP);

            if (_currIOCtlStateIndex == MOTOR_IOCTL_STATE_INDEX_STOP) {
                _motorState = MOTOR_STATE_STOP;
            } else {
                _motorState = MOTOR_STATE_TRANSITIONING;
                _transitionStartMillis = elapsed_millis;
            }
        }
    }
}

Motors::Motors() :
   _currIOCtlStateIndex(MOTOR_IOCTL_STATE_INDEX_STOP)
   , _newIOCtlStateIndex(_currIOCtlStateIndex)
   , _motorState(MOTOR_STATE_STOP)
   , _transitionStartMillis(0) {

    MaskAndPin *runner = maskAndPin;
    while (runner->mask != MOTOR_IOCTL_STATE_MASK_INVALID) {
        pinMode(runner->pin, OUTPUT);
        digitalWrite(runner->pin, LOW);
        runner++;
    }
}
Motors::~Motors() {
}

/*******************************************************************************************************************************/
class Device {
private:
public:
    unsigned long _status;
    Device();
    ~Device();
};

Device::Device() : _status(0) {
}

Device::~Device() {
}

/*******************************************************************************************************************************/
class TcpClient : public WiFiClient {
public:
    void begin_connect(const char *host, uint16_t port);
    bool isConnected();
};

bool TcpClient::isConnected() {return _client ? true : false;}

void TcpClient::begin_connect(const char *host, uint16_t port) {
    IPAddress ip;
    if (!WiFi.hostByName(host, ip)) {return;}

    ip_addr_t addr;
    addr.addr = ip;

    if (_client) stop();

    netif *interface = ip_route(&addr);
    if (!interface) {
        DEBUGV("no route to host\r\n");
        return;
    }

    tcp_pcb *pcb = tcp_new();
    if (!pcb) return;

    if (_localPort > 0) {
        pcb->local_port = _localPort++;
    }

    tcp_arg(pcb, this);
    tcp_err(pcb, &WiFiClient::_s_err);
    tcp_connect(pcb, &addr, port, reinterpret_cast<tcp_connected_fn>(&WiFiClient::_s_connected));
}

/*******************************************************************************************************************************/
class WifiActivity : public TimerFunction {
public:
    WifiActivity();
    virtual ~WifiActivity();
    virtual void begin() = 0;
    virtual void end() = 0;
};

WifiActivity::WifiActivity() { }
WifiActivity::~WifiActivity() { }

/*******************************************************************************************************************************/
class TcpConnection : public WifiActivity {
private:
    std::unique_ptr<TcpClient> _client;
public:
    TcpConnection();
    virtual ~TcpConnection();
    void run(unsigned long elapsed_millis);
    void begin();
    void end();
};

TcpConnection::TcpConnection() : _client(nullptr) { }
TcpConnection::~TcpConnection() { }

void TcpConnection::end() {
    delete _client.release();
}

void TcpConnection::begin() {
    _client.reset(new TcpClient());
    _client->begin_connect("192.168.254.104", 8888);
}

void TcpConnection::run(unsigned long elapsed_millis) {
    if (_client.get() == NULL) {return;}

    if (_client->isConnected()) {
        Serial.print("\nTcpConnection is connected!\n");
    }
}

/*******************************************************************************************************************************/
class WifiActivityListItem {
public:
    WifiActivity* _activity;
    std::unique_ptr<WifiActivityListItem> _next;

    WifiActivityListItem(WifiActivity* activity);
};

WifiActivityListItem::WifiActivityListItem(WifiActivity* activity) : _activity(activity), _next(nullptr) {}
/*******************************************************************************************************************************/
class WifiConnection : public TimerFunction {
private:
    typedef enum {
        WIFI_BEGIN_CONNECTING = 0,
        WIFI_WAITING_FOR_CONNECTION,
        WIFI_CONNECTED,
        WIFI_FAILED_TO_CONNECT
    } State;

    const char *_ssid;
    const char *_password;
    State _state;
    int _retries;
    StatusLed *_statusLed;
    std::unique_ptr<WifiActivityListItem> _activities;

    void initializeActivities();
    void deinitializeActivities();

    void run(unsigned long elapsed_millis);
public:
    void addActivity(WifiActivity* a);
    WifiConnection(StatusLed *statusLed);
    ~WifiConnection();
};

void WifiConnection::initializeActivities() {
    WifiActivityListItem* runner = _activities.get();

    while (runner != NULL) {
        runner->_activity->begin();
        runner = runner->_next.get();
    }
}

void WifiConnection::deinitializeActivities() {
    WifiActivityListItem* runner = _activities.get();

    while (runner != NULL) {
        runner->_activity->end();
        runner = runner->_next.get();
    }
}

void WifiConnection::addActivity(WifiActivity* a) {
    std::unique_ptr<WifiActivityListItem> w(new WifiActivityListItem(a));

    WifiActivityListItem* runner = _activities.get();
    if (runner == NULL) {
        _activities = std::move(w);
        return;
    }

    while (runner != NULL) {
        if (runner->_next.get() == NULL) {
            runner->_next = std::move(w);
            return;
        }
        runner = runner->_next.get();
    }
}

WifiConnection::WifiConnection(StatusLed *statusLed) : TimerFunction(),
   _ssid("AthenaEmilia0327"),
   _password("ttui987sde_"),
   _state(WIFI_BEGIN_CONNECTING),
   _retries(0),
   _statusLed(statusLed), 
    _activities(nullptr) {
}

WifiConnection::~WifiConnection() {
}

void WifiConnection::run(unsigned long elapsed_millis) {
    if (_state == WIFI_CONNECTED) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.print("Wifi no longer connected.\n");
            WiFi.disconnect(true);
            _state = WIFI_BEGIN_CONNECTING;
            _statusLed->_newState = StatusLed::LED_STATE_ERROR;
            deinitializeActivities();
        }
    } else if (_state == WIFI_BEGIN_CONNECTING) {
        WiFi.begin(_ssid, _password);
        _state = WIFI_WAITING_FOR_CONNECTION;
        _statusLed->_newState = StatusLed::LED_STATE_SEARCHING;
    } else if (_state == WIFI_WAITING_FOR_CONNECTION) {
        if (WiFi.isConnected()) {
            Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
            _state = WIFI_CONNECTED;
            _statusLed->_newState = StatusLed::LED_STATE_CONNECTED;
            _retries = 0;

            initializeActivities();
        } else {
            Serial.print("\nConnecting to ");
            Serial.println(_ssid);

            _retries++;
            if (_retries >= 10) {
                _state = WIFI_FAILED_TO_CONNECT;
                _retries = 0;
                _statusLed->_newState = StatusLed::LED_STATE_ERROR;
                Serial.print("\nFailed to connect!\n");
            }
        }
    } else if (_state == WIFI_FAILED_TO_CONNECT) {
        WiFi.disconnect(true);
        _state = WIFI_BEGIN_CONNECTING;
    }
}

/*******************************************************************************************************************************/

void setup(void) {
    static Device *dev = NULL;
    std::unique_ptr<TimerFunction> motors(new Motors());
    std::unique_ptr<TimerFunction> statusLed(new StatusLed());
    std::unique_ptr<WifiConnection> wifi(new WifiConnection(static_cast<StatusLed *>(statusLed.get())));

    std::unique_ptr<WifiActivity> tcpConnection(new TcpConnection());
    wifi->addActivity(tcpConnection.get());

    Serial.begin(SERIAL_BAUD_RATE);
    delay(5000);
    unsigned long elapsed_millis = millis();

    TimerSys::register_func(std::move(statusLed), 125, elapsed_millis);
    TimerSys::register_func(std::move(motors), 10, elapsed_millis);
    TimerSys::register_func(std::move(wifi), 1000, elapsed_millis);
    TimerSys::register_func(std::move(tcpConnection), 1000, elapsed_millis);
}

void loop(void) {
    TimerSys::update(millis());
}


