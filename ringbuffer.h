// simple interrupt-safe ring-buffer (assuming one reader and one writer)
// greatly inspired by the implementation found here:
// https://stackoverflow.com/questions/3020143/how-to-receive-packets-on-the-mcus-serial-port?rq=1

#ifndef RINGBUFFER_H_INCLUDED
#define RINGBUFFER_H_INCLUDED

#ifndef ARRAY_ELEMENTS
#define ARRAY_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))
#endif

struct ringbuffer {
    volatile size_t m_in;
    volatile size_t m_out;
    byte _raw_wbuffer[buffsize];
} wbuffer;

#define BUFFER_CALC_NEXT_IN \
        (wbuffer.m_in == buffsize - 1) ? 0 : (wbuffer.m_in + 1)

#define BUFFER_CALC_NEXT_OUT \
        (wbuffer.m_out == buffsize - 1) ? 0 : (wbuffer.m_out + 1)

#define BUFFER_INIT {wbuffer.m_in = wbuffer.m_out = 0;}

#define BUFFER_IS_EMPTY (wbuffer.m_in == wbuffer.m_out)
#define BUFFER_IS_FULL (BUFFER_CALC_NEXT_IN == wbuffer.m_out)

#define BUFFER_NEXT_IN \
    (wbuffer._raw_wbuffer + wbuffer.m_in)

#define BUFFER_NEXT_OUT \
    (wbuffer._raw_wbuffer + wbuffer.m_out)

#define BUFFER_WRITE_ADVANCE \
    wbuffer.m_in = BUFFER_CALC_NEXT_IN

#define BUFFER_READ_ADVANCE \
    wbuffer.m_out = BUFFER_CALC_NEXT_OUT

#define BUFFER_REWRITE(value) \
    *BUFFER_NEXT_IN = value

#define BUFFER_REWRITE_NEXT(value) \
    *(BUFFER_NEXT_IN+1) = value  /* nb THIS ONLY WORKS CORRECTLY IF BUFFSIZE IS A MULTIPLE OF TWO */

#define BUFFER_WRITE_PUSH(value) \
    *BUFFER_NEXT_IN = value; \
    BUFFER_WRITE_ADVANCE

#define BUFFER_READ_PEEK \
    *BUFFER_NEXT_OUT

#define BUFFER_READ_PEEK_NEXT \
    *(BUFFER_NEXT_OUT+1)  /* nb THIS ONLY WORKS CORRECTLY IF BUFFSIZE IS A MULTIPLE OF TWO */

#define BUFFER_READ_POP \
    *BUFFER_NEXT_OUTIN = value; \
    BUFFER_WRITE_ADVANCE

#define BUFFER_CAN_ACCEPT(n) \
    (wbuffer.m_in < wbuffer.m_out && wbuffer.m_in + n < wbuffer.m_out) \
    || (wbuffer.m_in > wbuffer.m_out && (wbuffer.m_in + n < buffsize || wbuffer.m_in + n > buffsize && wbuffer.m_in + n - buffsize < wbuffer.m_out))

#endif // RINGBUFFER_H_INCLUDED