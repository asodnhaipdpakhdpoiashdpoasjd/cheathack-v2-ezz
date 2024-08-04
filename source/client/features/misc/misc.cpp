#include "misc.h"
#include "../hitscan/hitscan.h"
#include "../../hooks/hooks.h"

void c_misc::run(c_user_cmd* cmd) {
	if (!ctx->m_local_player || !ctx->m_local_player->is_alive() || !ctx->m_local_weapon)
		return;

	if (config->m_misc.m_taunt_slide)
		taunt_slide(ctx->m_local_player, cmd);

	if (!ctx->m_local_player->is_taunting())
	{
		if (config->m_misc.m_bunnyhop)
			bunnyhop(ctx->m_local_player, cmd);

		if (config->m_misc.m_auto_strafer)
			auto_strafer(cmd);

		if (config->m_misc.m_fast_stop && !ctx->m_running_shift)
			fast_stop(cmd);
	}
}

void c_misc::setupPitch(c_user_cmd* cmd, int pMode) {
	int randValFour{ rand() % 4 };
	int shouldBool{ rand() % 2 };
	bool randBool{ shouldBool == 1 ? true : false };

	static int switchDelayed{ 0 };

	static bool bSwitch{ false };
	static int cSwitch{ 0 };

	bSwitch = !bSwitch;
	cSwitch++;
	switchDelayed++;

	if (cSwitch > 2) {
		cSwitch = 0;
	}

	else if (cSwitch < 0) {
		cSwitch = 0;
	}

	if (switchDelayed > 3)
		switchDelayed = 0;

	if (switchDelayed < 0)
		switchDelayed = 0;

	switch (pMode) {
	case 0: { //None
		return;
	}break;

	case 1: { //Auto
		switch (ctx->m_local_player->m_player_class()) {
		case CLASS_SCOUT: {
			switch (ctx->m_local_weapon->get_slot()) {
			case SLOT_PRIMARY:
			case SLOT_SECONDARY: {

				if (ctx->m_local_player->is_on_ground()) {

					if (config->m_hvh.m_fake_lag.m_fakelag) { //FakeLag

						if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) { //If even
							if (bSwitch) {
								cmd->m_view_angles.m_x = 89.0f; return;
							}

							else {
								cmd->m_view_angles.m_x = -271.0f; return;
							}
						}

						else { //If odd
							if (randBool) {
								cmd->m_view_angles.m_x = 89.0f; return;
							}

							else {
								cmd->m_view_angles.m_x = -271.0f; return;
							}
						}
					}

					else { //No FakeLag
						if (bSwitch) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = -271.0f; return;
						}
					}
				}

				else { //Not on ground
					if (ctx->m_local_player->m_velocity().length() > 271.0f) {
						if (config->m_hvh.m_fake_lag.m_fakelag) {
							if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) { //If even
								if (bSwitch) {
									cmd->m_view_angles.m_x = 271.0f; return;
								}

								else {
									cmd->m_view_angles.m_x = -271.0f; return;
								}
							}

							else { //If odd
								if (randBool) {
									cmd->m_view_angles.m_x = 271.0f; return;
								}

								else {
									cmd->m_view_angles.m_x = -271.0f; return;
								}
							}
						}

						else {
							c_misc::setupPitch(cmd, 7); //Random
						}
					}

					else { //If stopped in air
						if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) { //If even
							if (bSwitch) {
								cmd->m_view_angles.m_x = 89.0f; return;
							}

							else {
								cmd->m_view_angles.m_x = -271.0f; return;
							}
						}

						else { //If odd
							if (randBool) {
								cmd->m_view_angles.m_x = 89.0f; return;
							}

							else {
								cmd->m_view_angles.m_x = -271.0f; return;
							}
						}
					}
				}
			}break;

			case SLOT_MELEE: {
				if (config->m_hvh.m_fake_lag.m_fakelag) { //FakeLag
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) { //If even
						if (bSwitch) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = -271.0f; return;
						}
					}

					else { //If odd
						if (randBool) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = -271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = 89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = -271.0f; return;
					}
				}
			}break;

			default:
				return;
			}
		}break;

		case CLASS_SOLDIER: {
			switch (ctx->m_local_weapon->get_slot()) {
			case SLOT_PRIMARY: {
				if (config->m_hvh.m_fake_lag.m_fakelag) { //FakeLag

					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) { //If even
						if (bSwitch) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}

					else { //If odd
						if (randBool) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}
				}

				else {
					if (randBool) {
						cmd->m_view_angles.m_x = -271.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = 271.0f; return;
					}
				}
			}break;

			case SLOT_SECONDARY:
			case SLOT_MELEE: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) { //If even
						if (bSwitch) { //Jitter Fake Down
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}

					else { //If odd
						if (randBool) { //Random Fake Down
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = -271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = 89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = -271.0f; return;
					}
				}
			}break;

			default:
				return;
			}
		}break;

		case CLASS_PYRO: {
			switch (ctx->m_local_weapon->get_slot()) {
			case SLOT_PRIMARY:
			case SLOT_MELEE: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = -271.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = -271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = 89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = -271.0f; return;
					}
				}
			}break;

			case SLOT_SECONDARY: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = -271.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = 271.0f; return;
					}
				}
			}break;

			default: {
				return;
			}break;
			}
		}break;

		case CLASS_DEMOMAN: {
			switch (ctx->m_local_weapon->get_slot()) {
			case SLOT_PRIMARY:
			case SLOT_SECONDARY: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = -271.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = -271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = 89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = -271.0f; return;
					}
				}
			}break;

			case SLOT_MELEE: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}
				}

				else {
					switch (switchDelayed) {
					case 0: {
						cmd->m_view_angles.m_x = -89.0f; return;
					}break;

					case 1: {
						cmd->m_view_angles.m_x = -271.0f; return;
					}break;

					case 2: {
						cmd->m_view_angles.m_x = 89.0f; return;
					}break;

					case 3: {
						cmd->m_view_angles.m_x = 271.0f; return;
					}

					default: {
						return;
					}break;
					}
				}
			}break;

			default: {
				return;
			}break;
			}
		}break;

		case CLASS_HEAVY: {
			switch (ctx->m_local_weapon->get_slot()) {
			case SLOT_PRIMARY: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = -271.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = 271.0f; return;
					}
				}
			}break;

			case SLOT_SECONDARY:
			case SLOT_MELEE: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 89.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 89.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = 89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = -271.0f; return;
					}
				}
			}break;

			default: {
				return;
			}break;
			}
		}break;

		case CLASS_ENGINEER: {
			if (config->m_hvh.m_fake_lag.m_fakelag) { //FakeLag

				if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) { //If even
					if (bSwitch) {
						cmd->m_view_angles.m_x = 89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = -271.0f; return;
					}
				}

				else { //If odd
					if (randBool) {
						cmd->m_view_angles.m_x = 89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = -271.0f; return;
					}
				}
			}

			else { //No FakeLag
				if (bSwitch) {
					cmd->m_view_angles.m_x = 89.0f; return;
				}

				else {
					cmd->m_view_angles.m_x = -271.0f; return;
				}
			}
		}

		case CLASS_MEDIC: {
			switch (ctx->m_local_weapon->get_slot()) {
			case SLOT_PRIMARY:
			case SLOT_MELEE: {
				if (config->m_hvh.m_fake_lag.m_fakelag) { //FakeLag

					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) { //If even
						if (bSwitch) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = -271.0f; return;
						}
					}

					else { //If odd
						if (randBool) {
							cmd->m_view_angles.m_x = 89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = -271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = 89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = -271.0f; return;
					}
				}
			}break;

			case SLOT_SECONDARY: {
				if (config->m_hvh.m_fake_lag.m_fakelag) { //FakeLag

					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) { //If even
						if (bSwitch) {
							cmd->m_view_angles.m_x = -89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}

					else { //If odd
						if (randBool) {
							cmd->m_view_angles.m_x = -89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = -89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = 271.0f; return;
					}
				}
			}break;

			default: {
				return;
			}break;
			}
		}break;

		case CLASS_SNIPER: {
			switch (ctx->m_local_weapon->get_slot()) {
			case SLOT_PRIMARY:
			case SLOT_SECONDARY: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = -89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = -89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = -89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = 271.0f; return;
					}
				}
			}break;

			case SLOT_MELEE: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = -271.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = 271.0f; return;
					}
				}
			}

			default: {
				return;
			}break;
			}
		}break;

		case CLASS_SPY: {
			switch (ctx->m_local_weapon->get_slot()) {
			case SLOT_PRIMARY: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = -89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = -89.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = -89.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = 271.0f; return;
					}
				}
			}break;

			case SLOT_SECONDARY: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 271.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = -271.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = 271.0f; return;
					}
				}
			}break;

			case SLOT_MELEE: {
				if (config->m_hvh.m_fake_lag.m_fakelag) {
					if (config->m_hvh.m_fake_lag.m_max_ticks % 2 == 0) {
						if (bSwitch) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 89.0f; return;
						}
					}

					else {
						if (randBool) {
							cmd->m_view_angles.m_x = -271.0f; return;
						}

						else {
							cmd->m_view_angles.m_x = 89.0f; return;
						}
					}
				}

				else {
					if (bSwitch) {
						cmd->m_view_angles.m_x = -271.0f; return;
					}

					else {
						cmd->m_view_angles.m_x = 89.0f; return;
					}
				}
			}break;

			default: {
				return;
			}break;
			}
		}break;

		default: {
			return;
		}break;
		}
	}break;

	case 2: { //Zero
		cmd->m_view_angles.m_x = 0.0f; return;
	}break;

	case 3: { //Up
		cmd->m_view_angles.m_x = -89.0f; return;
	}break;

	case 4: { //Down
		cmd->m_view_angles.m_x = 89.0f; return;
	}break;

	case 5: { //Fake Up
		cmd->m_view_angles.m_x = -271.0f; return;
	}break;

	case 6: { //Fake Down
		cmd->m_view_angles.m_x = 271.0f; return;
	}break;

	case 7: { //Random
		switch (randValFour) {
		case 0: {
			cmd->m_view_angles.m_x = -89.0f; return;
		}break;

		case 1: {
			cmd->m_view_angles.m_x = 89.0f; return;
		}break;

		case 2: {
			cmd->m_view_angles.m_x = -271.0f; return;
		}break;

		case 3: {
			cmd->m_view_angles.m_x = 271.0f; return;
		}break;

		default:
			return;
		}
	}break;

	case 8: { //Real Random
		if (randBool) {
			cmd->m_view_angles.m_x = -89.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = 89.0f; return;
		}
	}break;

	case 9: { //Fake Random
		if (randBool) {
			cmd->m_view_angles.m_x = -271.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = 271.0f; return;
		}
	}break;

	case 10: { //Random Fake Up
		if (randBool) {
			cmd->m_view_angles.m_x = -89.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = -271.0f; return;
		}
	}break;

	case 11: { //Random Fake Down
		if (randBool) {
			cmd->m_view_angles.m_x = 89.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = 271.0f; return;
		}
	}break;

	case 12: { //Up Fake Random
		if (randBool) {
			cmd->m_view_angles.m_x = -89.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = 271.0f; return;
		}
	}break;

	case 13: { //Down Fake Random
		if (randBool) {
			cmd->m_view_angles.m_x = 89.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = -271.0f; return;
		}
	}break;

	case 14: { //Jitter
		if (bSwitch) {
			cmd->m_view_angles.m_x = -89.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = 89.0f; return;
		}
	}break;

	case 15: { //Fake Jitter
		if (bSwitch) {
			cmd->m_view_angles.m_x = -271.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = 271.0f; return;
		}
	}break;

	case 16: { //Jitter Fake Up
		if (bSwitch) {
			cmd->m_view_angles.m_x = -89.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = -271.0f; return;
		}
	}break;

	case 17: { //Jitter Fake Down
		if (bSwitch) {
			cmd->m_view_angles.m_x = 89.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = 271.0f; return;
		}
	}break;

	case 18: { //Up Fake Jitter
		if (bSwitch) {
			cmd->m_view_angles.m_x = -89.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = 271.0f; return;
		}
	}break;

	case 19: { //Down Fake Jitter
		if (bSwitch) {
			cmd->m_view_angles.m_x = 89.0f; return;
		}

		else {
			cmd->m_view_angles.m_x = -271.0f; return;
		}
	}break;

	case 20: { //Custom
		if (!config->m_hvh.m_aa_pitch.djPitch) {
			cmd->m_view_angles.m_x = config->m_hvh.m_aa_pitch.mainPitch; return;
		}

		else {
			if (config->m_hvh.m_aa_pitch.djPitch) {
				if (!config->m_hvh.m_aa_pitch.tjPitch) {
					if (bSwitch) {
						cmd->m_view_angles.m_x = config->m_hvh.m_aa_pitch.firstJit; return;
					}

					else {
						cmd->m_view_angles.m_x = config->m_hvh.m_aa_pitch.secoundJit; return;
					}
				}

				else {
					switch (cSwitch) {
					case 0: {
						cmd->m_view_angles.m_x = config->m_hvh.m_aa_pitch.firstJit; return;
					}break;

					case 1: {
						cmd->m_view_angles.m_x = config->m_hvh.m_aa_pitch.secoundJit; return;
					}break;

					case 2: {
						cmd->m_view_angles.m_x = config->m_hvh.m_aa_pitch.thirdJit; return;
					}break;

					default: {
						return;
					}break;
					}
				}
			}
		}
	}break;

	default: {
		return;
	}break;
	}
}

void c_misc::setupYaw(c_user_cmd* cmd, int yaw, bool rORf) { //rORf(true / real, false / fake), sSType(0 / left, 1 / right)
	static float rSS{ 0.0f };
	static float fSS{ 0.0f };

	// We have a value from 0 to 20, that is our spin speed
	// We want to check for if we're currently spinning left or right, depending on it we want to put it negative or positive
	// We have some solutions

	if (rORf) {
		if (yaw == 4) {
			rSS += config->m_hvh.m_aa_rYaw.rSS;
		}

		else if (yaw == 5) {
			rSS -= config->m_hvh.m_aa_rYaw.rSS;
		}
	}

	else {
		if (yaw == 4) {
			fSS += config->m_hvh.m_aa_fYaw.fSS;
		}

		else {
			fSS -= config->m_hvh.m_aa_fYaw.fSS;
		}
	}

	if (rSS > 180.0f)
		rSS = -180.0f;

	if (rSS < -180.0f)
		rSS = 180.0f;

	if (fSS > 180.0f)
		fSS = -180.0f;

	if (fSS < -180.0f)
		fSS = 180.0f;

	switch (yaw) {
	case 0: { //Forward
		cmd->m_view_angles.m_y += 0.0f; return;
	}break;

	case 1: { //Left
		cmd->m_view_angles.m_y += 90.0f; return;
	}break;

	case 2: { //Right
		cmd->m_view_angles.m_y -= 90.0f; return;
	}break;

	case 3: { //Backwards
		cmd->m_view_angles.m_y += 180.0f; return;
	}break;

	case 4: { //Spin Left
		rORf ? cmd->m_view_angles.m_y = rSS : cmd->m_view_angles.m_y = fSS; return;
	}break;

	case 5: { //Spin Right
		rORf ? cmd->m_view_angles.m_y = rSS : cmd->m_view_angles.m_y = fSS; return;
	}break;

	default: {
		return;
	}break;
	}
}

void c_misc::anti_aim(c_user_cmd* cmd, bool* send_packet) {
	if (!ctx->m_local_player || !ctx->m_local_player->is_alive() || !ctx->m_local_weapon)
		return;

	if (ctx->m_local_player->is_ghost() || ctx->m_local_player->is_charging() || ctx->m_local_player->is_taunting())
		return;

	if ((ctx->m_local_weapon->can_shoot() && ctx->m_local_weapon->get_ammo() > 0) && ctx->m_local_weapon->is_attacking(cmd) && cmd->m_buttons & IN_ATTACK)
		return;

	if (ctx->m_running_shift || ctx->m_shift)
		return;

	if (!config->m_hvh.m_fake_lag.m_fakelag && ((config->m_hvh.m_anti_aim.m_masterS && config->m_hvh.m_anti_aim.m_yaws_master) && config->m_hvh.m_anti_aim.m_fYaw_master)) {
		if (ctx->m_choke_tick)
			ctx->m_choke_tick = !ctx->m_choke_tick;

		else
			ctx->m_choke_tick = !ctx->m_choke_tick;

		*send_packet = ctx->m_choke_tick;
	}

	if (config->m_hvh.m_anti_aim.m_masterS && config->m_hvh.m_anti_aim.m_pitch_master) {
		c_misc::setupPitch(cmd, config->m_hvh.m_aa_pitch.pitch);
	}

	if ((config->m_hvh.m_anti_aim.m_masterS && config->m_hvh.m_anti_aim.m_yaws_master && config->m_hvh.m_anti_aim.m_fYaw_master) && *send_packet == true) {
		//send_packet need to be false
		c_misc::setupYaw(cmd, config->m_hvh.m_aa_fYaw.fYaw, false);
	}

	else if ((config->m_hvh.m_anti_aim.m_masterS && config->m_hvh.m_anti_aim.m_yaws_master && config->m_hvh.m_anti_aim.m_rYaw_master) && *send_packet == false) {
		//send_packet need to be false
		c_misc::setupYaw(cmd, config->m_hvh.m_aa_rYaw.rYaw, true);
	}
}

/*void c_misc::anti_aim(c_user_cmd* cmd, bool* send_packet) {
	if ( !ctx->m_local_player || !ctx->m_local_player->is_alive( ) || !ctx->m_local_weapon )
		return;

	if ( ctx->m_running_shift || ctx->m_shift)
		return;

	auto old_angles = cmd->m_view_angles;

	c_base_player* best_player = hitscan->closest_to_crosshair;
	static bool side = false;
	bool found_local = false;
	if (best_player)
	{
		vector left, right, forward;
		math::angle_vectors(ctx->m_engine_client->get_view_angles() - vector(0.0f, 90.0f, 0.0f), &left);
		math::angle_vectors(ctx->m_engine_client->get_view_angles() + vector(0.0f, 90.0f, 0.0f), &right);

		left = ctx->m_local_player->get_world_space_center() + left * 65;
		right = ctx->m_local_player->get_world_space_center() + right * 65;

		if (tf2::is_visible(best_player, ctx->m_local_player, best_player->get_world_space_center(), left, MASK_SHOT | CONTENTS_GRATE))
		{
			side = false;
			found_local = true;
		}

		if (tf2::is_visible(best_player, ctx->m_local_player, best_player->get_world_space_center(), right, MASK_SHOT | CONTENTS_GRATE))
		{
			side = true;
			found_local = true;
		}
	}

	static int pitch_swap_delay = 0;
	static int real_swap_delay = 0;
	static int fake_swap_delay = 0;

	if (config->m_hvh.m_pitch.m_enabled)
	{
		static float pitch = 69.f;
		static bool swap = false;
		if (config->m_hvh.m_pitch.m_jitter)
		{
			if (pitch_swap_delay-- <= 0)
			{
				swap = !swap;
				pitch = swap ? config->m_hvh.m_pitch.m_min_angle : config->m_hvh.m_pitch.m_max_angle;
				pitch_swap_delay = config->m_hvh.m_pitch.m_jitter_max;
			}
		}
		else
		{
			pitch = config->m_hvh.m_pitch.m_pitch_angle;
		}

		cmd->m_view_angles.m_x = pitch;
	}

	ctx->m_real_angles.m_x = ctx->m_fake_angles.m_x = cmd->m_view_angles.m_x;

	static int angle_multiplier = -1;

	float fake_yaw = cmd->m_view_angles.m_y;
	float real_yaw = cmd->m_view_angles.m_y;

	if (!config->m_hvh.m_fakelag && ( !config->m_hvh.m_fake_yaw.m_enabled || !config->m_hvh.m_real_yaw.m_enabled ) || ctx->m_unchoke_del > 1 ) {
		if (ctx->m_choke_tick)
			ctx->m_choke_tick = false;

		else
			ctx->m_choke_tick = true;

		*send_packet = ctx->m_choke_tick;
	}

	if (config->m_hvh.m_fake_yaw.m_enabled && *send_packet == true)
	{
		fake_yaw += !side ? 90.f : -90.f;

		if (config->m_hvh.m_fake_yaw.m_jitter)
		{
			if (fake_swap_delay-- <= 0)
			{
				static int current_angle_index = 0;
				std::vector<float> angles = {};

				for (int n = 1; n <= config->m_hvh.m_fake_yaw.m_jitter_max + 1; n++)
				{
					angles.emplace_back((config->m_hvh.m_fake_yaw.m_min_angle / n) * 10);
					angles.emplace_back((config->m_hvh.m_fake_yaw.m_max_angle / n) * 10);
				}

				if (static_cast<size_t>(current_angle_index++) >= angles.size() - 1u)
					current_angle_index = 0;

				fake_yaw += angles[current_angle_index];
				fake_swap_delay = 3; // yessir
			}
		}

		if (config->m_hvh.m_fake_yaw.m_anti_logic)
		{
			fake_yaw += ctx->m_globals->m_current_time * 630.f;
			while (fake_yaw > 180.f)
				fake_yaw -= 360.f;

			while (fake_yaw < -180.f)
				fake_yaw += 360.f;
		}

		cmd->m_view_angles.m_y += fake_yaw;
		ctx->m_fake_angles = cmd->m_view_angles;
	}

	if (config->m_hvh.m_real_yaw.m_enabled && *send_packet == false)
	{
		if (config->m_hvh.m_real_yaw.m_edge)
		{
			real_yaw -= side ? -90.f : 90.f;
		}
		else if (config->m_hvh.m_real_yaw.m_anti_logic)
		{
			if (real_swap_delay-- <= 0)
			{
				if (side)
					angle_multiplier = 1.f;
				else
					angle_multiplier = -1.f;

				real_swap_delay = 8; // yessir
			}
		}

		cmd->m_view_angles.m_y += std::clamp(real_yaw, -180.f, 180.f);
		ctx->m_real_angles = cmd->m_view_angles;

		if (!ctx->m_local_player->is_taunting())
			leg_jitter(cmd);
	}

	if ( ( ctx->m_local_weapon->get_weapon_id( ) == TF_WEAPON_PIPEBOMBLAUNCHER || ctx->m_local_weapon->get_weapon_id( ) == TF_WEAPON_COMPOUND_BOW ) && ctx->m_local_weapon->m_charge_begin_time( ) > 0.0f )
		cmd->m_view_angles = old_angles;

	else if ( cmd->m_buttons & IN_ATTACK && ctx->m_local_weapon->can_shoot( ) )
		cmd->m_view_angles = old_angles;
}*/

/*void c_misc::fake_lag(c_user_cmd* cmd, int minticks, bool* send_packet) {
	if (!config->m_hvh.m_fake_lag.m_fakelag)
		return;

	if (ctx->m_shift || ctx->m_running_shift || ctx->m_is_recharging)
		return;

	//We need to recreate the lag compensation.
	//First we calculate the velocity per tick, doing so:
	float velPerTick{ ctx->m_local_player->m_velocity().length_2d() * ctx->m_globals->m_interval_per_tick };

	// Then we check if velPerTick is <= 0, if so.. Set it to 1?
	if (velPerTick <= 0)
		velPerTick = 1;

	//First we get the lag compensation, that is our velocity per tick divided by the server's tickrate.
	int lagComp{ std::clamp((int)std::ceilf(64.0f / velPerTick), 0, 22) };

	//Then we set our maxticks easily
	int maxticks{ config->m_hvh.m_fake_lag.m_lag_comp ? (lagComp > minticks ? minticks : lagComp) : minticks };

	if (ctx->m_charged_ticks > 2)
		maxticks = 0;

	static bool isIdle{ false };

	if (ctx->m_local_player->m_velocity().length() < 2)
		isIdle = true;

	else
		isIdle = false;

	static bool isAttacking{ false };
	if (ctx->m_local_weapon->is_attacking(cmd)) {
		isAttacking = true;
	}

	else {
		isAttacking = false;
	}

	static bool flLocked{ false };

	if (isIdle)
		flLocked = false;

	if (ctx->m_client_state->m_choked_commands > (maxticks >= 22 ? 21 : maxticks)) {
		*send_packet = true;
		ctx->m_choking = false;
	}

	else {
		*send_packet = false;
		ctx->m_choking = true;
	}
}*/

void c_misc::fake_lag(c_user_cmd* cmd, int maxticks, bool* send_packet) {
	if (ctx->m_shift || ctx->m_running_shift || !config->m_hvh.m_fake_lag.m_fakelag || ctx->m_is_recharging) {
		ctx->m_choking = false;
		return;
	}

	if (!ctx->m_local_player || !ctx->m_local_player->is_alive() || ctx->m_local_player->is_taunting()) {
		ctx->m_choking = false;
		return;
	}

	if (ctx->m_local_weapon->is_attacking(cmd)) {
		*send_packet = true;
		ctx->m_choking = false;
		return;
	}

	float velocity_per_tick = std::max(ctx->m_local_player->m_velocity().length_2d() * ctx->m_globals->m_interval_per_tick, 1.f);

	int lag_comp_required = std::clamp(static_cast<int>(ceilf(64.f / velocity_per_tick)), 0, maxticks);

	int min_ticks = config->m_hvh.m_fake_lag.m_lag_comp ? lag_comp_required : maxticks;

	if (ctx->m_charged_ticks > 2) {
		min_ticks = 0;
	}

	*send_packet = ctx->m_client_state->m_choked_commands >= min_ticks;
	ctx->m_choking = !*send_packet;

}

void c_misc::leg_jitter(c_user_cmd* cmd) {
	if (!ctx->m_local_player || !config->m_hvh.m_anti_aim.m_leg_jitter)
		return;

	static bool pos = false;
	float speedMultiplier = ctx->m_local_player->m_ducked() ? 3.75f : 1.25f;

	if (cmd->m_forward_move == 0.f && cmd->m_side_move == 0.f && ctx->m_local_player->m_velocity().length_2d() < 10.f) {
		const float yaw = DEG_TO_RAD(fmodf(ctx->m_globals->m_current_time * 360.f * 1.95f, 360.f));
		const float velDir = yaw;
		const float wishAng = atan2f(-cmd->m_side_move, cmd->m_forward_move);
		const float delta = math::angle_diff_rad(velDir, wishAng);
		const float moveDir = delta < 0.0f ? velDir : velDir;

		cmd->m_forward_move = cosf(moveDir) * speedMultiplier;
		cmd->m_side_move = -sinf(moveDir) * speedMultiplier;

		pos = !pos;
	}
}

void c_misc::handle_dt(c_user_cmd* cmd, bool* send_packet) {
	if (!ctx->m_local_player || !ctx->m_local_weapon || (!(ctx->m_local_player->m_flags() & FL_ONGROUND) && !config->m_exploits.m_doubletap_in_air))
		return;

	if (!tf2::is_allowed_to_dt(ctx->m_local_weapon))
		return;

	if (ctx->m_warping) {
		cmd->m_buttons &= ~IN_ATTACK;
		return;
	}

	if (!ctx->m_shift && !ctx->m_running_shift) {
		static c_base_combat_weapon* old_weapon = nullptr;

		if (old_weapon != ctx->m_local_weapon) {
			ctx->m_wait_ticks = 24;
			old_weapon = ctx->m_local_weapon;
		}

		if (!ctx->m_local_weapon->can_shoot() && ctx->m_shifted_ticks <= 0)
			ctx->m_wait_ticks = 24;

		if (ctx->m_local_player->m_player_class() == CLASS_HEAVY) {
			if (ctx->m_local_weapon->get_slot() == SLOT_PRIMARY && ctx->m_local_weapon->m_minigun_state() != AC_STATE_SPINNING)
				ctx->m_wait_ticks = 24;
		}
	}

	if (ctx->m_wait_ticks > 0)
		return;

	if (cmd->m_buttons & IN_ATTACK && ctx->m_local_weapon->can_shoot()) {
		auto allowed_ticks = config->m_misc.m_reduce_input_delay ? 20 : ((ctx->m_local_weapon->m_item_definition_index() == Scout_m_ForceANature ||
			ctx->m_local_weapon->m_item_definition_index() == Scout_m_FestiveForceANature) ? 21 : 22);

		if (ctx->m_charged_ticks >= allowed_ticks) {
			*send_packet = false;
			ctx->m_shift = true;
		}
	}
}

void c_misc::bunnyhop(c_base_player* player, c_user_cmd* cmd) {
	static bool jump_state = false;

	if (cmd->m_buttons & IN_JUMP) {
		if (!jump_state && !(player->m_flags() & FL_ONGROUND))
			cmd->m_buttons &= ~IN_JUMP;

		else if (jump_state)
			jump_state = false;
	}

	else if (!jump_state)
		jump_state = true;
}

/*void c_misc::velocity_handler(c_user_cmd* cmd) {
	if (!config->m_aimbot.m_delayshot)
		return;

	if (ctx->m_local_player && ctx->m_running_shift && !ctx->m_warping) {
		if (ctx->m_charged_ticks > (config->m_misc.m_reduce_input_delay ? 6 : 14)) {
			auto velocity = ctx->m_local_player->m_velocity();
			auto angles = vector();
			auto forward = vector();

			math::vector_angles(velocity, angles);
			angles.m_y = math::angle_diff_rad(cmd->m_view_angles.m_y, angles.m_y);
			math::angle_vectors(angles, &forward);
			forward *= velocity.length();

			cmd->m_forward_move = -forward.m_x * (config->m_misc.m_reduce_input_delay ? 2.0f : 1.0f);
			cmd->m_side_move = -forward.m_y * (config->m_misc.m_reduce_input_delay ? 2.0f : 1.0f);
		}
		else if (ctx->m_charged_ticks > (config->m_misc.m_reduce_input_delay ? 1 : 3)) {
			cmd->m_forward_move = cmd->m_side_move = 0.0f;
			cmd->m_buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
		}
	}
}*/

// default
void c_misc::velocity_handler(c_user_cmd* cmd) {
	if (!config->m_aimbot.m_delayshot)
		return;

	if (ctx->m_local_player && ctx->m_running_shift && !ctx->m_warping) {
		if (ctx->m_charged_ticks > (config->m_misc.m_reduce_input_delay ? 6 : 14)) {
			auto angles = vector();
			auto forward = vector();
			math::vector_angles(ctx->m_local_player->m_velocity(), angles);
			angles.m_y = cmd->m_view_angles.m_y - angles.m_y;
			math::angle_vectors(angles, &forward);
			forward *= ctx->m_local_player->m_velocity().length();

			cmd->m_forward_move = -forward.m_x * (config->m_misc.m_reduce_input_delay ? 2.f : 1.f);
			cmd->m_side_move = -forward.m_y * (config->m_misc.m_reduce_input_delay ? 2.f : 1.f);
		}
		else if (ctx->m_charged_ticks > (config->m_misc.m_reduce_input_delay ? 1 : 3)) {
			cmd->m_forward_move = cmd->m_side_move = 0.0f;
			cmd->m_buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
		}
	}
}

// gaza bombing not tested too lazy to test lmao + i think need small fixes
/*void c_misc::gaza(c_user_cmd* cmd) {
	if ((GetAsyncKeyState(config->m_projectile_aim->GazaBombingKey) & 0x8000) && INPUT->is_window_in_focus() && !ctx->m_surface->is_cursor_visible())
	{
		float CurrentCharge = ctx->m_globals->m_current_time - ctx->m_local_weapon->m_charge_begin_time();
		float MaxCharge = 0.0037f;

		if (ctx->m_local_weapon->m_item_definition_index() == Demoman_s_TheQuickiebombLauncher)
			MaxCharge = 0.014f;

		if (CurrentCharge > 0.1f)
			return;

		if (CurrentCharge >= MaxCharge)
			cmd->m_buttons &= ~IN_ATTACK;

		g->Doubletap.Recharge = true;
	}
}*/

void c_misc::auto_strafer(c_user_cmd* cmd) {
	static bool was_jumping = false;
	bool is_jumping = cmd->m_buttons & IN_JUMP;

	if (!ctx->m_local_player || !ctx->m_local_player->is_alive() || ctx->m_local_player->m_water_level() > 1)
		return;

	if (ctx->m_local_player->m_move_type() == MOVETYPE_NOCLIP
		|| ctx->m_local_player->m_move_type() == MOVETYPE_LADDER
		|| ctx->m_local_player->m_move_type() == MOVETYPE_OBSERVER)
	{
		return;
	}

	if (!(ctx->m_local_player->m_flags() & FL_ONGROUND) && (!is_jumping || was_jumping)) {
		if (ctx->m_local_player->m_flags() & (1 << 11))
			return;

		const float speed = ctx->m_local_player->m_velocity().length_2d();
		auto& velocity = ctx->m_local_player->m_velocity();

		if (speed < 2.0f)
			return;

		const auto compute_perfect_delta = [](float speed) noexcept {
			static auto air_acceleration = ctx->m_cvar->find_var("sv_airaccelerate");

			if (!air_acceleration)
				return 0.0f;

			const auto term = 25.f / air_acceleration->get_float() / powf(ctx->m_local_player->m_max_speed(), 2.0f) / speed;

			if (term < 1.0f && term > -1.0f)
				return acosf(term);

			return 0.f;
		};

		if (const auto delta = compute_perfect_delta(speed)) {
			const auto yaw = DEG_TO_RAD(ctx->m_engine_client->get_view_angles().m_y);
			const auto velocity_direction = atan2f(velocity.m_y, velocity.m_x) - yaw;
			const auto wish_angle = atan2f(-cmd->m_side_move, cmd->m_forward_move);
			const auto delta_ = math::angle_diff_rad(velocity_direction, wish_angle);
			const auto move_direction = delta_ < 0.0f ? velocity_direction + delta : velocity_direction - delta;

			cmd->m_forward_move = cosf(move_direction) * 450.f;
			cmd->m_side_move = -sinf(move_direction) * 450.f;
		}
	}

	was_jumping = is_jumping;
}

void c_misc::fast_stop(c_user_cmd* cmd) {
	if (cmd->m_buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))
		return;

	if (!ctx->m_local_player || !ctx->m_local_player->is_alive())
		return;

	if (ctx->m_local_player->m_flags() & FL_ONGROUND) {
		const auto speed = ctx->m_local_player->m_velocity().length();
		if (speed > 20.f) {
			vector direction;
			math::vector_angles(ctx->m_local_player->m_velocity(), direction);
			direction.m_y = cmd->m_view_angles.m_y - direction.m_y;
			vector forward;
			math::angle_vectors(direction, &forward);

			vector negated_direction = forward * speed;

			cmd->m_forward_move = -negated_direction.m_x;
			cmd->m_side_move = -negated_direction.m_y;
		}
	}
}

void c_misc::taunt_slide(c_base_player* player, c_user_cmd* cmd) {
	if (player->m_player_cond() & TFCond_Taunting) {
		float forward = 0.0f, side = 0.0f;

		if (cmd->m_buttons & IN_FORWARD)
			forward += 450.f;

		if (cmd->m_buttons & IN_BACK) {
			side -= 450.f;
			cmd->m_view_angles.m_z = -90.f;
		}

		if (cmd->m_buttons & IN_MOVELEFT)
			side -= 450.f;

		if (cmd->m_buttons & IN_MOVERIGHT)
			side += 450.f;

		if ((!(cmd->m_buttons & IN_FORWARD) && !(cmd->m_buttons & IN_BACK) && !(cmd->m_buttons & IN_MOVELEFT) && !(cmd->m_buttons & IN_MOVERIGHT))) {
			forward -= 450.f;
			cmd->m_view_angles.m_x = 90.f;
		}

		cmd->m_forward_move = forward;
		cmd->m_side_move = side;
		cmd->m_view_angles.m_y = ctx->m_engine_client->get_view_angles().m_y;
	}
}

bool c_misc::handle_warp(float extra_sample, bool final_tick, std::function<void(float, bool)> f) {
	if (!config->m_exploits.m_warp)
		return false;

	int ticks_per_call = config->m_exploits.m_warp_speed;

	if (config->m_exploits.m_dynamic_warp_ticks) {
		float velocity_per_tick = ctx->m_local_player->m_velocity().length_2d() * ctx->m_globals->m_interval_per_tick;
		velocity_per_tick = std::max(velocity_per_tick, 1.f);

		int lag_comp_required = std::clamp(static_cast<int>(std::ceil(64.f / velocity_per_tick)), 0, 22);
		ticks_per_call = lag_comp_required / 2;
	}

	if (config->m_exploits.m_tickbase_warp_key.is_held() && ctx->m_charged_ticks >= ticks_per_call) {
		for (int i = 0; i < ticks_per_call; i++) {
			f(extra_sample, final_tick); // cl_move call
			ctx->m_charged_ticks--;
			ctx->m_shifted_ticks++;
			ctx->m_warping = true;

			ctx->m_dt_recharge_wait = ctx->m_globals->m_current_time + 0.2f;
			*ctx->m_send_packet = (i == ticks_per_call - 1);
		}
		ctx->m_warping = false;
	}

	return false;
}

void c_misc::auto_warp(c_user_cmd* cmd) {
	if (!ctx->m_local_player || !ctx->m_local_player->is_alive() || !ctx->m_local_weapon || config->m_hvh.m_fake_lag.m_fakelag || ctx->m_charged_ticks <= 0)
		return;

	if (!config->m_exploits.m_auto_warp || !config->m_exploits.m_auto_warp_key.is_held())
		return;

	std::vector< std::pair< float, c_base_player* > > targets;

	for (const auto& player : ctx->players) {
		if (!player || !player->is_player() || !player->is_alive() || !player->is_enemy() || player->is_invulnerable() || player->is_dormant() || player->is_ghost())
			continue;

		auto fov = math::calculate_fov(ctx->m_engine_client->get_view_angles(), math::calculate_angle(ctx->m_local_player->get_world_space_center(), player->get_world_space_center()));

		if (fov > 90.f)
			continue;

		targets.emplace_back(std::pair< float, c_base_player* >(fov, player));
	}

	std::sort(targets.begin(), targets.end(), [](const std::pair< float, c_base_player* >& a, const std::pair< float, c_base_player* >& b) {
		return a.first > b.first;
		});

	while (targets.size() > static_cast<size_t>(config->m_aimbot.m_max_targets))
		targets.pop_back();

	bool side = true;
	bool can_see_player = false;
	vector left, right, forward;
	math::angle_vectors(ctx->m_engine_client->get_view_angles() - vector(0.0f, 90.0f, 0.0f), &left);
	math::angle_vectors(ctx->m_engine_client->get_view_angles() + vector(0.0f, 90.0f, 0.0f), &right);

	left = ctx->m_local_player->get_world_space_center() + left * config->m_exploits.m_auto_warp_scale;
	right = ctx->m_local_player->get_world_space_center() + right * config->m_exploits.m_auto_warp_scale;

	for (auto& target : targets) {
		if (tf2::is_visible(target.second, ctx->m_local_player, target.second->get_world_space_center(), left, MASK_SHOT | CONTENTS_GRATE)) {
			side = false;
			can_see_player = true;
		}

		else if (tf2::is_visible(target.second, ctx->m_local_player, target.second->get_world_space_center(), right, MASK_SHOT | CONTENTS_GRATE)) {
			side = true;
			can_see_player = true;
		}

		if (can_see_player)
			break;
	}

	if (can_see_player && !(cmd->m_buttons & IN_ATTACK)) {
		cmd->m_side_move = (side ? -450 : 450);

		if (ctx->m_local_player->m_velocity().length() > 100.f) {
			ctx->m_shift = true;
		}
	}
}

void c_misc::handle_weapon_sway() {
	static auto cl_wpn_sway_interp = ctx->m_cvar->find_var("cl_wpn_sway_interp");
	static auto cl_wpn_sway_scale = ctx->m_cvar->find_var("cl_wpn_sway_scale");

	if (cl_wpn_sway_interp && cl_wpn_sway_scale) {
		static const float old_sway_interp = cl_wpn_sway_interp->get_float();
		static const float old_sway_scale = cl_wpn_sway_scale->get_float();

		if (config->m_visuals.m_viewmodel_sway) {
			cl_wpn_sway_interp->set_value(0.1f);
			cl_wpn_sway_scale->set_value(config->m_visuals.m_viewmodel_sway_val);
		}

		else {
			cl_wpn_sway_interp->set_value(old_sway_interp);
			cl_wpn_sway_scale->set_value(old_sway_scale);
		}
	}
}

void c_misc::fix_local_animations() {
	auto local = ctx->m_local_player;
	if (!local || !local->is_alive())
		return;

	if (local->m_force_taunt_cam()) {
		ctx->m_prediction->set_local_view_angles(ctx->m_real_angles);

		// Replace with fake chams
		// bool old_m_client_side_animations = local->m_client_side_animations();

		// int choked = ctx->m_client_state->m_choked_commands;
		// if (choked > 1) {
		//     local->m_client_side_animations() = false;
		//     local->m_eye_angles() = ctx->m_fake_angles;
		// } else {
		//     local->m_client_side_animations() = true;
		// }

		local->m_client_side_animations() = true;
	}
}

void c_misc::anti_backstab(c_user_cmd* cmd) {
	if (!config->m_hvh.m_anti_aim.m_anti_backstab || !ctx->m_local_player || !ctx->m_local_player->is_alive() || !ctx->m_local_weapon ||
		(cmd->m_buttons & IN_ATTACK && ctx->m_local_weapon->can_shoot()) || ctx->m_running_shift)
		return;

	float best_distance = 110.f;
	c_base_player* best_spy = nullptr;

	for (const auto& player : ctx->players) {
		if (!player || !player->is_player() || !player->is_alive() || player->is_dormant() ||
			player->m_player_class() != CLASS_SPY || !player->is_enemy())
			continue;

		float distance = player->m_origin().dist_to(ctx->m_local_player->m_origin());
		if (distance > 135.f)
			continue;

		if (distance < best_distance) {
			best_distance = distance;
			best_spy = player;
		}
	}

	if (best_spy) {
		vector target_angle = math::calculate_angle(ctx->m_local_player->get_shoot_position(), best_spy->get_world_space_center());
		float fov_to_target = math::calculate_fov(ctx->m_engine_client->get_view_angles(), target_angle);

		if (fov_to_target > 35.f) {
			if (!config->m_hvh.m_fake_lag.m_fakelag)
				*ctx->m_send_packet = ctx->m_client_state->m_choked_commands >= 2;

			if (*ctx->m_send_packet)
				cmd->m_view_angles.m_y = target_angle.m_y;
		}
	}
}

void c_misc::scout_jump(c_user_cmd* cmd) {
	static bool shot = false;
	if (!ctx->m_local_player || !ctx->m_local_player->is_alive() || !ctx->m_local_weapon || ctx->m_local_player->m_water_level() > 1)
		return;

	if (ctx->m_local_player->m_move_type() == MOVETYPE_NOCLIP || ctx->m_local_player->m_move_type() == MOVETYPE_LADDER || ctx->m_local_player->m_move_type() == MOVETYPE_OBSERVER)
		return;

	if (util::is_window_in_focus() && (GetAsyncKeyState(config->m_misc.m_scout_jump_key.m_key) & 1) && !ctx->m_surface->is_cursor_visible())
	{
		if (ctx->m_local_player && ctx->m_local_player->is_alive() && !ctx->m_local_player->is_dormant() && ctx->m_local_player->m_player_class() == CLASS_SCOUT && ctx->m_local_player->is_on_ground())
		{
			if (ctx->m_local_weapon->m_item_definition_index() == Scout_m_ForceANature || ctx->m_local_weapon->m_item_definition_index() == Scout_m_FestiveForceANature)
			{
				vector angle = { 37.f, cmd->m_view_angles.m_y, 0.0f };
				if (!shot)
				{
					cmd->m_buttons |= IN_JUMP;
					cmd->m_buttons |= IN_ATTACK;
					shot = true;
				}
				shot = false;

				tf2::fix_movement(cmd, angle);
				cmd->m_view_angles = angle;
			}
		}
	}
}

void c_misc::fix_input_delay(bool final_tick) {
	if (ctx->m_client_state->m_net_channel && ctx->m_client_state->m_net_channel->is_loopback())
		return;

	if (!ctx->m_engine_client->is_in_game())
		return;

	globals_backup backup{};
	backup.copy();

	hooks::cl_read_packets::original_function(final_tick);

	backup.restore();
}


bool c_misc::should_read_packets() {
	if (!ctx->m_client_state->m_net_channel || ctx->m_client_state->m_net_channel->is_loopback())
		return true;

	if (!ctx->m_engine_client->is_in_game())
		return true;

	// state.restore();

	return false;
}

void c_misc::shoot_sticky_at_charge(c_user_cmd* cmd) {
	if (!ctx->m_local_player || !ctx->m_local_weapon ||
		ctx->m_local_player->m_player_class() != CLASS_DEMOMAN || ctx->m_local_weapon->get_slot() != 1 ||
		(config->m_aimbot_global.m_enabled && config->m_projectile_aim.m_enabled && config->m_aimbot_global.m_aim_key.is_held()) ||
		config->m_automations.m_charge_max == 0 || !(cmd->m_buttons & IN_ATTACK))
		return;

	float current_charge = ctx->m_globals->m_current_time - ctx->m_local_weapon->m_charge_begin_time();
	float max_charge_factor = (ctx->m_local_weapon->m_item_definition_index() == Demoman_s_TheQuickiebombLauncher) ? 70.0f : 27.0f;
	float max_charge = config->m_automations.m_charge_max / max_charge_factor;
	float latency = ctx->m_engine_client->get_net_channel_info()->get_latency(0);

	if (current_charge > 5.0f || current_charge >= max_charge)
		cmd->m_buttons &= ~IN_ATTACK;
}

// rule 7. :jesuspng:
void bye_ping() {
	static con_var* reduce_ping = ctx->m_cvar->find_var("cl_cmdrate");
	if (reduce_ping) {
		reduce_ping->m_name = "cl_cmdrate";
		reduce_ping->set_value(-1);
	}
}

/*void c_misc::shoot_sticky_at_charge(c_user_cmd* cmd) {
	if (!ctx->m_local_player || !ctx->m_local_weapon ||
		!(ctx->m_local_player->m_player_class() == CLASS_DEMOMAN && ctx->m_local_weapon->get_slot() == 1) ||
		(config->m_aimbot_global.m_enabled && config->m_projectile_aim.m_enabled && config->m_aimbot_global.m_aim_key.is_held()) ||
		config->m_automations.m_charge_max == 0 || !(cmd->m_buttons & IN_ATTACK))
		return;

	float get_current_charge = ctx->m_globals->m_current_time - ctx->m_local_weapon->m_charge_begin_time();
	float get_max_charge = (ctx->m_local_weapon->m_item_definition_index() == Demoman_s_TheQuickiebombLauncher)
		? config->m_automations.m_charge_max / 70.0f
		: config->m_automations.m_charge_max / 27.0f;
	float get_latency = ctx->m_engine_client->get_net_channel_info()->get_latency(0);

	if (get_current_charge > 5.0f || get_current_charge >= get_max_charge)
		cmd->m_buttons &= ~IN_ATTACK;
}*/



