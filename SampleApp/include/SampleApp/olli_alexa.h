#ifndef OLLI_ALEEXA_H_
#define OLLI_ALEEXA_H_

#ifdef __cplusplus
extern "C"
{
#endif

enum alexa_state { 
    /// Alexa is idle and ready for an interaction.
    OLLI_ALEXA_IDLE,

    /// Alexa is currently listening.
    OLLI_ALEXA_LISTENING,

    /// Alexa is currently expecting a response from the customer.
    OLLI_ALEXA_EXPECTING,

    /**
     * A customer request has been completed and no more input is accepted. In this state, Alexa is waiting for a
     * response from AVS.
     */
    OLLI_ALEXA_THINKING,

    /// Alexa is responding to a request with speech.
    OLLI_ALEXA_SPEAKING,

    /**
     * Alexa has finished processing a SPEAK directive. In this state there
     * are no notifications triggered. If the SPEAK directive is part of a
     * speech burst UX moves back to the SPEAKING state. If it was the last
     * SPEAK directive after timeout the UX state moves to the IDLE state.
     */
    OLLI_ALEXA_FINISHED,
    OLLI_ALEXA_CONTROL,
};

struct olli_msg
{
    enum alexa_state state;
    char* str_data;
    int* int_data;
};

void olli_report_state(enum alexa_state state);
void olli_work_loop_read_data(void (*cb_func)(void *msg, void *ptr), void *user_data);

#ifdef __cplusplus
}
#endif

#endif