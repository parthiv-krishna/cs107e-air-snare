#include <ampi.h>
#include <ampienv.h>

#include <linux/synchronize.h>
#include "gpio.h"
#include "gpioextra.h"
#include "common.h"
#include "malloc.h"
#include "assert.h"
#include "printf.h"
#include "interrupts.h"
#include "timer.h"
#include "mailbox.h"
#include "armtimer.h"
#include "LSM6DS33.h"
#include "config.h"

void *ampi_malloc(size_t size)
{
	// Ensure malloc doesn't get interrupted
	// EnterCritical() keeps track of whether the interrupts are currently
	// enabled or disabled so that when we LeaveCritical() we reset the state
	// rather than always assuming we should re-enable
	linuxemu_EnterCritical();
	void *ret = malloc(size);
	linuxemu_LeaveCritical();
	return ret;
}

void ampi_free(void *ptr)
{
	linuxemu_EnterCritical();
	free(ptr);
	linuxemu_LeaveCritical();
}

void ampi_assertion_failed(const char *pExpr, const char *pFile, unsigned nLine)
{
	printf("Assertion failed: %s. File %s, line %d\n", pExpr, pFile, nLine);
	assert(false);
}

void MsDelay(unsigned ms)
{
	timer_delay_ms(ms);
}

void usDelay(unsigned us)
{
	timer_delay_us(us);
}

static TPeriodicTimerHandler *m_pHandlerTimer = NULL;

static bool alarm(unsigned int pc)
{
	if (!armtimer_check_and_clear_interrupt())
		return false;
	if (m_pHandlerTimer != NULL)
		m_pHandlerTimer();
	AMPiPoke();
	return true;
}

void RegisterPeriodicHandler(TPeriodicTimerHandler *pHandler)
{
	assert(m_pHandlerTimer == NULL);
	m_pHandlerTimer = pHandler;
}

static const int m_nIRQ = INTERRUPTS_BASIC_ARM_DOORBELL_0_IRQ;
static TInterruptHandler *m_pHandler = NULL;
static void *paramToPass = NULL;
static bool handleConnectInterrupt(unsigned int pc)
{
	m_pHandler(paramToPass);
	return true;
}

void ConnectInterrupt(unsigned nIRQ, TInterruptHandler *pHandler, void *pParam)
{
	assert(paramToPass == NULL && m_pHandler == NULL);
	assert(nIRQ == m_nIRQ);
	m_pHandler = pHandler;
	paramToPass = pParam;
	interrupts_register_handler(nIRQ, handleConnectInterrupt);
	interrupts_enable_source(nIRQ);
}

void LogWrite(const char *pSource, unsigned Severity, const char *pMessage, ...)
{
	static const char *logLevels[] = {"UNKNOWN", "ERROR", "WARNING", "NOTICE", "DEBUG"};
	va_list args;
	va_start(args, pMessage);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), pMessage, args);
	printf("%s (source: %s): %s\n", logLevels[Severity], pSource, buf);
	va_end(args);
}

uint32_t EnableVCHIQ(uint32_t buf)
{
	typedef mbox_buf(4) proptag;
	proptag *mboxbuf = (proptag *)0x4c80000;
	mbox_init(*mboxbuf);
	mboxbuf->tag.id = 0x48010;
	mboxbuf->tag.u32[0] = buf;
	mailbox_write(MAILBOX_TAGS_ARM_TO_VC, (unsigned int)mboxbuf);
	mailbox_read(MAILBOX_TAGS_ARM_TO_VC);
	return mboxbuf->tag.u32[0];
}

void *GetCoherentRegion512K(void)
{
	return (void *)0x4c00000;
}

extern void InitializeMMU(void);

void env_init(void)
{
#ifndef DEBUG_NO_AUDIO
	InitializeMMU();
#endif
	interrupts_init();
	uart_init();


	armtimer_init(10000); // 10 000 us = 100 Hz
	armtimer_enable();
	interrupts_register_handler(INTERRUPTS_BASIC_ARM_TIMER_IRQ, alarm);
	armtimer_enable_interrupts();
	interrupts_enable_source(INTERRUPTS_BASIC_ARM_TIMER_IRQ);

#ifndef DEBUG_NO_AUDIO
	interrupts_global_enable();
#endif

	// IRQs
	// uint32_t *irq = (uint32_t *)0x18;
	// *irq = 0xea000000 | ((uint32_t *)_irq_stub - irq - 2);
	// __asm__ __volatile__ ("cpsie i");

	// // Timer
	// *SYSTMR_C1 = *SYSTMR_CLO + T1_INTV;
	// ConnectInterrupt(1, timer1_handler, NULL);
}