#include <stdlib.h>
#include <errno.h>

#include "dcpu_keyboard.h"
#include "dcpu_vcpu.h"

struct dcpu_hw_ops dcpu_keyboard_ops;

// create a new keyboard and return it
struct dcpu_keyboard * dcpu_keyboard_new(void)
{
	struct dcpu_keyboard *kbd;

	kbd = calloc(1, sizeof(*kbd));

	dcpu_hw_init(&kbd->hw);

	kbd->hw.hw_version   = DCPU_KEYBOARD_VERSION;
	kbd->hw.hw_vendor_id = DCPU_KEYBOARD_VENDOR_ID;
	kbd->hw.hw_device_id = DCPU_KEYBOARD_DEVICE_ID;

	dcpu_fifo_init(&kbd->buffer, DCPU_MAX_KEYS_QUEUED);
	kbd->int_message = 0;

	kbd->hw.ops = dcpu_keyboard_ops;

	return kbd;

}

// create a new keyboard and register it with a vcpu
// returns the hw_id or negative on error
int dcpu_add_keyboard(struct dcpu_vcpu *vcpu)
{
	struct dcpu_keyboard *kbd;
	int rc;

	kbd = dcpu_keyboard_new();
	if (!kbd)
		return -ENOMEM;

	rc = dcpu_vcpu_register_hw(vcpu, &kbd->hw);
	if (rc >= 0)
		return kbd->hw.hw_id;

	dcpu_vcpu_unregister_hw(vcpu, &kbd->hw);

	kbd->hw.ops.delete(&kbd->hw);

	return rc;
}

static int dcpu_keyboard_poke (struct dcpu_hw *hw)
{
	struct dcpu_keyboard *kbd = dcpu_hw_to_keyboard(hw);
	//struct dcpu_vcpu *vcpu = kbd->hw.vcpu;
	dcpu_word key = 0;

	//key = getchar();
	dcpu_fifo_put(&kbd->buffer, key);

	return 0;
}

static int dcpu_keyboard_handle_interrupt (struct dcpu_hw *hw)
{
	struct dcpu_keyboard *kbd = dcpu_hw_to_keyboard(hw);
	struct dcpu_vcpu *vcpu = kbd->hw.vcpu;
	dcpu_word key, b;

	switch (dcpu_vcpu_gr(vcpu)->a) {
	case DCPU_KEYBOARD_CMD_CLEAR:  // clear keyboard buffer
		dcpu_fifo_reset(&kbd->buffer);
		break;

	case DCPU_KEYBOARD_CMD_GETKEY:  // read key in buffer into C
		if (! dcpu_fifo_get(&kbd->buffer, &key))
			key = 0;
		dcpu_vcpu_gr(vcpu)->c = key;
		break;

	case DCPU_KEYBOARD_CMD_TESTKEY:  // if B is pushed, set C to 1

		// TODO: this needs a major fixing.
		b = dcpu_vcpu_gr(vcpu)->b;
		if (dcpu_fifo_get(&kbd->buffer, &key) && key == b)
			dcpu_vcpu_gr(vcpu)->c = 1;
		else
			dcpu_vcpu_gr(vcpu)->c = 0;
		break;

	case DCPU_KEYBOARD_CMD_IE:  // interrupt enable
		kbd->int_message = dcpu_vcpu_gr(vcpu)->b;
		break;

	default:
		// ignore
		break;
	}

	return 0;
}

static void dcpu_keyboard_delete (struct dcpu_hw *hw)
{
	struct dcpu_keyboard *kbd = dcpu_hw_to_keyboard(hw);

	free(kbd);
}

struct dcpu_hw_ops dcpu_keyboard_ops = {
	.poke = dcpu_keyboard_poke,
	.handle_interrupt = dcpu_keyboard_handle_interrupt,
	.delete = dcpu_keyboard_delete,
};

