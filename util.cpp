#if 1
#include "common.h"
Corbot::SerialType Serial;
#else

template<typename _Tp1>
struct auto_ptr_ref {
    _Tp1 *_M_ptr;

    explicit
    auto_ptr_ref(_Tp1 *__p) : _M_ptr(__p) { }
};

template<typename _Tp>
class auto_ptr {
private:
    _Tp *_p;

public:
    typedef _Tp element_type;
    explicit auto_ptr(element_type *__p = 0) throw() : _p(__p) {
    }

    auto_ptr(auto_ptr &__a) throw() : _p(__a.release()) {
    }

    template<typename _Tp1>
    auto_ptr(auto_ptr<_Tp1> &__a) throw() : _p(__a.release()) {
    }

    auto_ptr& operator=(auto_ptr &__a) throw() {
        reset(__a.release());
        return *this;
    }

    template<typename _Tp1>
    auto_ptr& operator=(auto_ptr<_Tp1> &__a) throw() {
        reset(__a.release());
        return *this;
    }

    ~auto_ptr() {
        delete _p;
    }

    element_type& operator*() const throw() {
        return *_p;
    }

    element_type * operator->() const throw() {
        return _p;
    }

    element_type* get() const throw() {
        return _p;
    }

    element_type* release() throw() {
        element_type *__tmp = _p;
        _p = 0;
        return __tmp;
    }

    void reset(element_type *__p = 0) throw() {
        if (__p != _p) {
            delete _p;
            _p = __p;
        }
    }

    auto_ptr(auto_ptr_ref<element_type> __ref) throw()
        : _p(__ref._M_ptr) { }

    auto_ptr& operator=(auto_ptr_ref<element_type> __ref) throw() {
        if (__ref._M_ptr != this->get()) {
            delete _p;
            _p = __ref._M_ptr;
        }
        return *this;
    }

    template<typename _Tp1>
    operator auto_ptr_ref<_Tp1>() throw() {
        return auto_ptr_ref<_Tp1>(this->release());
    }

    template<typename _Tp1>
    operator auto_ptr<_Tp1>() throw() {
        return auto_ptr<_Tp1>(this->release());
    }
};

template<>
class auto_ptr<void> {
public:
    typedef void element_type;
};

void setup() {
}

void loop() {
}


#if defined(LINUX_COMPILE)
    #include "common.h"
#endif

void sleep_device() {
#if defined(LINUX_COMPILE)
    int rc;
    struct timeval timeout;
    memset(&timeout, 0x00, sizeof(struct timeval));
    timeout.tv_usec = 10;
    timeout.tv_sec = 0;

    rc = select(0, NULL, NULL, NULL, &timeout);
    if (rc == -1) {
        return;
    }
#endif
}

/*****************************************************************
    CLASS DECLARATION 
*****************************************************************/
#if 1
template <class T>
class auto_ptr {
private:
    T* _t;
public:
    auto_ptr() : _t(0) {
    }

    auto_ptr(T* t) : _t(t) {
    }

    /*auto_ptr(auto_ptr<T>& p) : _t(0) {
        _t = p.release();
    }*/

    auto_ptr<T> operator=(auto_ptr<T>& p) {
        if (_t) {
            delete _t;
        }

        _t = p.release();
    }

    T* operator->() {
        return this->get();
    }

    ~auto_ptr() {
        if(_t) {
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

#else

template<class T>
class auto_ptr {
private:
    T* _t;

public:
    auto_ptr() : _t(0) {
    }
    auto_ptr(T*__p = 0) : _t(__p) {
    }

    ~auto_ptr() {
        delete _t;
    }

    T * operator->() {
        return _t;
    }

    T* get() {
        return _t;
    }

    T* release() {
        T *__tmp = _t;
        _t = 0;
        return __tmp;
    }
};

#endif

class Element {
public:
    Element* _next;
    Element();
    virtual ~Element();
};

class TimerFunction : public Element {
private:
    unsigned long _interval;
    unsigned long _lastRun;
public:
    TimerFunction(unsigned long interval);
    ~TimerFunction();
    void update(unsigned long elapsed_millis);
    virtual void run() = 0;
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
    int size();
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

class TimerSys {
private:
    static ListElem _functions;
public:
    static bool register_func(auto_ptr<TimerFunction> tf);
    static void update(unsigned long elapsed_millis);
};

/*****************************************************************
    CLASS BODY 
*****************************************************************/

Element::Element() : _next(0) {
}


Element::~Element() {
}

ListElem::ListElem() : _head(0) {
}

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
    auto_ptr<ListIterator> r (new ListIterator(this));

    return r;
}

ListIterator::ListIterator(ListElem* owner) : _owner(owner), _index(0) {

}

ListIterator::~ListIterator() {
}

Element* ListIterator::next() {
    Element *runner = _owner->_head; 
    int i = 0;
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
    int i = 0;
    while (runner != NULL) {
        if (i == _index) {
            return true;
        }

        i++;
        runner = runner->_next;
    }

    return false;
        Element* tmp = _owner->_head;
}

bool TimerSys::register_func(auto_ptr<TimerFunction> tf) { 
    auto_ptr<Element> t (tf.release());
    return _functions.add(t);
}


void TimerSys::update(unsigned long elapsed_millis) {
    auto_ptr<ListIterator> i = _functions.iterator();
    /*ListIterator* it = _functions.iterator().release();
    auto_ptr<ListIterator> i(it);*/

    while (i->hasNext()) {
        TimerFunction* tf = static_cast<TimerFunction*>(i->next());
        tf->update(elapsed_millis);
    }
}

TimerFunction::TimerFunction(unsigned long interval) : _interval(interval), _lastRun(0) {
}

TimerFunction::~TimerFunction() {
}

void TimerFunction::update(unsigned long elapsed_millis) {
    if (_lastRun == 0) {
        _lastRun = elapsed_millis;
        return;
    } else {
        if ((elapsed_millis - _lastRun) > _interval) {
            _lastRun = elapsed_millis;
            run();
        }
    }
}

void setup(void) {
  Serial.begin(230400);

  //TimerSys::register_func(UpdateLED, 10);
}

void loop(void) {
    sleep_device();

    unsigned long m_sec = millis();

    Serial.print("m_sec = ");
    Serial.print(m_sec);
    Serial.print("\n");
    delay(1000);
    fprintf(stderr, "m_sec = %ld\n", m_sec);
}

#endif
