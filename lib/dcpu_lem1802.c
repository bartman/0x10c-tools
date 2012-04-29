#include <stdlib.h>
#include <errno.h>

#include "dcpu_lem1802.h"
#include "dcpu_vcpu.h"

static dcpu_word dcpu_lem1802_default_font[DCPU_LEM1802_FONT_RAM_SIZE] = {
	0 /* TODO: put something here */
};
static dcpu_word dcpu_lem1802_default_palette[DCPU_LEM1802_PALETTE_RAM_SIZE] = {
	0 /* TODO: put something here */
};

struct dcpu_hw_ops dcpu_lem1802_ops;

// create a new lem1802 and return it
struct dcpu_lem1802 * dcpu_lem1802_new(void)
{
	struct dcpu_lem1802 *lem;

	lem = calloc(1, sizeof(*lem));

	dcpu_hw_init(&lem->hw);

	lem->hw.hw_version   = DCPU_LEM1802_VERSION;
	lem->hw.hw_vendor_id = DCPU_LEM1802_VENDOR_ID;
	lem->hw.hw_device_id = DCPU_LEM1802_DEVICE_ID;

	lem->video_ram_base = 0;
	lem->font_ram_base = 0;
	lem->palette_ram_base = 0;

	lem->hw.ops = dcpu_lem1802_ops;

	return lem;

}

// create a new lem1802 and register it with a vcpu
// returns the hw_id or negative on error
int dcpu_add_lem1802(struct dcpu_vcpu *vcpu)
{
	struct dcpu_lem1802 *lem;
	int rc;

	lem = dcpu_lem1802_new();
	if (!lem)
		return -ENOMEM;

	rc = dcpu_vcpu_register_hw(vcpu, &lem->hw);
	if (rc >= 0)
		return lem->hw.hw_id;

	dcpu_vcpu_unregister_hw(vcpu, &lem->hw);

	lem->hw.ops.delete(&lem->hw);

	return rc;
}

static int dcpu_lem1802_poke (struct dcpu_hw *hw)
{
	/* should do something like update the display */

	return 0;
}

static int dcpu_lem1802_handle_interrupt (struct dcpu_hw *hw)
{
	struct dcpu_lem1802 *lem = dcpu_hw_to_lem1802(hw);
	struct dcpu_vcpu *vcpu = lem->hw.vcpu;
	dcpu_cycles_t b;

	switch (dcpu_vcpu_gr(vcpu)->a) {
	case DCPU_LEM1802_CMD_MEM_MAP_SCREEN:  // maps 386 words from B as video RAM
		b = dcpu_vcpu_gr(vcpu)->b;
		lem->video_ram_base = b;
		break;

	case DCPU_LEM1802_CMD_MEM_MAP_FONT:  // maps 256 words from B as font RAM
		b = dcpu_vcpu_gr(vcpu)->b;
		lem->font_ram_base = b;
		break;

	case DCPU_LEM1802_CMD_MEM_MAP_PALETTE:  // maps 16 words from B as palette RAM
		b = dcpu_vcpu_gr(vcpu)->b;
		lem->palette_ram_base = b;
		break;

	case DCPU_LEM1802_CMD_SET_BORDER_COLOR:  // sets border to B & 0xF
		b = dcpu_vcpu_gr(vcpu)->b;
		lem->border_color = b & 0xF;
		break;

	case DCPU_LEM1802_CMD_MEM_DUMP_FONT:  // copies 386 words of font data to [B]
		b = dcpu_vcpu_gr(vcpu)->b;
		dcpu_vcpu_copy_to_ram(vcpu, b, dcpu_lem1802_default_font,
				DCPU_LEM1802_FONT_RAM_SIZE);
		break;

	case DCPU_LEM1802_CMD_MEM_DUMP_PALETTE:  // copies 16 words of palette data to [B]
		b = dcpu_vcpu_gr(vcpu)->b;
		dcpu_vcpu_copy_to_ram(vcpu, b, dcpu_lem1802_default_palette,
				DCPU_LEM1802_PALETTE_RAM_SIZE);
		break;

	default:
		// ignore
		break;
	}

	return 0;
}

static void dcpu_lem1802_delete (struct dcpu_hw *hw)
{
	struct dcpu_lem1802 *lem = dcpu_hw_to_lem1802(hw);

	free(lem);
}

struct dcpu_hw_ops dcpu_lem1802_ops = {
	.poke = dcpu_lem1802_poke,
	.handle_interrupt = dcpu_lem1802_handle_interrupt,
	.delete = dcpu_lem1802_delete,
};

