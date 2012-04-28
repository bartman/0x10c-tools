#include <stdlib.h>
#include <errno.h>

#include "dcpu_clock.h"
#include "dcpu_vcpu.h"

struct dcpu_hw_ops dcpu_clock_ops;

// create a new clock and return it
struct dcpu_clock * dcpu_clock_new(void)
{
	struct dcpu_clock *clock;

	clock = calloc(1, sizeof(*clock));

	clock->cycles_per_tick = 0;
	clock->int_message = 0;
	clock->ticks = 0;
	clock->last_conf_cycles = -1;
	clock->last_tick_cycles = -1;

	clock->hw.ops = dcpu_clock_ops;

	return clock;

}

// create a new clock and register it with a vcpu
// returns the hw_id or negative on error
int dcpu_add_clock(struct dcpu_vcpu *vcpu)
{
	struct dcpu_clock *clock;
	int rc;

	clock = dcpu_clock_new();
	if (!clock)
		return -ENOMEM;

	rc = dcpu_vcpu_register_hw(vcpu, &clock->hw);
	if (rc >= 0)
		return clock->hw.hw_id;

	dcpu_vcpu_unregister_hw(vcpu, &clock->hw);

	clock->hw.ops.delete(&clock->hw);

	return rc;
}

static int dcpu_clock_poke (struct dcpu_hw *hw)
{
	struct dcpu_clock *clock = dcpu_hw_to_clock(hw);
	struct dcpu_vcpu *vcpu = clock->hw.vcpu;

	if (!clock->cycles_per_tick)
		return 0;

	dcpu_cycles_t now = dcpu_vcpu_cycles(vcpu);

	if (now < (clock->last_tick_cycles + clock->cycles_per_tick))
		return 0;

	int64_t diff = now - clock->last_tick_cycles;
	int32_t missed = diff / clock->cycles_per_tick;

	clock->ticks += missed;
	clock->last_tick_cycles += missed * clock->cycles_per_tick;

	if (clock->int_message) {
		int i;
		for (i=0; i<missed; i++)
			dcpu_vcpu_accept_interrupt(vcpu, clock->int_message);
	}

	return 0;
}

static int dcpu_clock_handle_interrupt (struct dcpu_hw *hw)
{
	struct dcpu_clock *clock = dcpu_hw_to_clock(hw);
	struct dcpu_vcpu *vcpu = clock->hw.vcpu;
	dcpu_cycles_t b;

	switch (dcpu_vcpu_gr(vcpu)->a) {
	case DCPU_CLOCK_CMD_CONF:  // configure
		b = dcpu_vcpu_gr(vcpu)->b;

		clock->last_conf_cycles = dcpu_vcpu_cycles(vcpu);

		if (b) {
			// configure and enable clock
			clock->cycles_per_tick = (dcpu_cycles_t)vcpu->hz
				* b / 60;

			clock->ticks = 0;
			clock->last_tick_cycles = clock->last_conf_cycles;

		} else {
			// disable clock
			clock->cycles_per_tick = 0;
			clock->last_tick_cycles = -1;
		}
		break;

	case DCPU_CLOCK_CMD_READ:  // read counter

		dcpu_vcpu_gr(vcpu)->c = clock->ticks;
		break;

	case DCPU_CLOCK_CMD_IE:  // interrupt enable

		clock->int_message = dcpu_vcpu_gr(vcpu)->b;
		break;

	default:
		// ignore
		break;
	}

	return 0;
}

static void dcpu_clock_delete (struct dcpu_hw *hw)
{
	struct dcpu_clock *clock = dcpu_hw_to_clock(hw);

	free(clock);
}

struct dcpu_hw_ops dcpu_clock_ops = {
	.poke = dcpu_clock_poke,
	.handle_interrupt = dcpu_clock_handle_interrupt,
	.delete = dcpu_clock_delete,
};

