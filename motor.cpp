#define SERIAL_BAUD_RATE              230400

#include <ESP8266WiFi.h>

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

template<class T>
class auto_ptr {
private:
    T* _t;
public:
    auto_ptr() : _t(0) {
    }

    auto_ptr(T* t) : _t(t) {
    }

    template<typename _T1>
    auto_ptr(auto_ptr<_T1> &__a) : _t(__a.release()) {
    }

    auto_ptr<T> operator=(auto_ptr<T> &p) {
        if (_t) {
            delete _t;
        }

        _t = p.release();
    }

    T* operator->() {
        return this->get();
    }

    ~auto_ptr() {
        if (_t) {
            delete _t;
            _t = NULL;
        }
    }

    T* get() const {
        return _t;
    }

    T* release() {
        T* tmp = _t;
        _t = NULL;

        return tmp;
    }
};


class Element {
public:
    Element* _next;
    Element();
    virtual ~Element();
};

class ListIterator;
class ListElem : public Element {
private:
    Element* _head;
    friend class ListIterator;
public:
    ListElem();
    ~ListElem();
    bool add(auto_ptr<Element>);
    auto_ptr<ListIterator> iterator();
};


class ListIterator {
private:
    ListElem* _owner;
    unsigned int _index;
public:
    ListIterator(ListElem* owner);
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
    static bool register_func(auto_ptr<TimerFunction> tf, unsigned long interval, unsigned long elapsed_millis);
    static void update(unsigned long elapsed_millis);
};

ListElem TimerSys::_functions;

Element::Element() : _next(0) { }
Element::~Element() { }

/*******************************************************************************************************************************/
ListElem::ListElem() : _head(0) { }

ListElem::~ListElem() {
    while (_head != NULL) {
        Element* tmp = _head;
        _head = _head->_next;

        delete tmp;
    }
}

bool ListElem::add(auto_ptr<Element> e) {
    if (e.get() == NULL) {
        return false;
    }

    if (_head == NULL) {
        _head = e.release();
        _head->_next = NULL;
    } else {
        Element* runner = _head;
        while (runner->_next != NULL) {
            runner = runner->_next;
        }

        runner->_next = e.release();
        runner->_next->_next = NULL;
    }

    return true;
}

auto_ptr<ListIterator> ListElem::iterator() {
    auto_ptr<ListIterator> r(new ListIterator(this));

    return r;
}

/*******************************************************************************************************************************/
ListIterator::ListIterator(ListElem* owner) : _owner(owner), _index(0) { }
ListIterator::~ListIterator() { }

Element* ListIterator::next() {
    Element* runner = _owner->_head;
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
    Element* runner = _owner->_head;
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

bool TimerSys::register_func(auto_ptr<TimerFunction> tf, unsigned long interval, unsigned long elapsed_millis) {
    tf->setInterval(interval, elapsed_millis);
    auto_ptr<Element> t(tf.release());
    return TimerSys::_functions.add(t);
}

void TimerSys::update(unsigned long elapsed_millis) {
    auto_ptr<ListIterator> i = TimerSys::_functions.iterator();

    while (i->hasNext()) {
        TimerFunction* tf = static_cast<TimerFunction*>(i->next());
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
    {LED_STATE_CONNECTED, 0x8000}
    , {LED_STATE_ERROR, 0xAAAA}
    , {LED_STATE_SEARCHING, 0xF0F0}
    , {LED_STATE_AP_MODE, 0xA000}
    , {LED_STATE_INVALID, 0xFFFF}
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
      {MOTOR_IOCTL_STATE_MASK_1, D1}
    , {MOTOR_IOCTL_STATE_MASK_2, D2}
    , {MOTOR_IOCTL_STATE_MASK_3, D5}
    , {MOTOR_IOCTL_STATE_MASK_4, D6}
    , {MOTOR_IOCTL_STATE_MASK_INVALID, 0}
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

    MaskAndPin* runner = maskAndPin;
    while (runner->mask != MOTOR_IOCTL_STATE_MASK_INVALID) {
        digitalWrite(runner->pin, (_ioctlStates[stateIndex] & runner->mask) == 0 ? LOW : HIGH);
        runner++;
    }
}

void Motors::run(unsigned long elapsed_millis) {
#if 1
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
    ,_newIOCtlStateIndex(_currIOCtlStateIndex)
   , _motorState(MOTOR_STATE_STOP)
   , _transitionStartMillis(0) {

    MaskAndPin* runner = maskAndPin;
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


/*******************************************************************************************************************************/

void setup(void) { 
    static Device* dev = NULL;
    auto_ptr<TimerFunction> motors(new Motors());
    auto_ptr<TimerFunction> statusLed(new StatusLed());

    Serial.begin(SERIAL_BAUD_RATE);
    delay(2000);
    unsigned long elapsed_millis = millis();



    TimerSys::register_func(statusLed, 125, elapsed_millis);
    TimerSys::register_func(motors, 10, elapsed_millis);
}

void loop(void) {
    TimerSys::update(millis());
}



