/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "client.h"
#include "game.h"
#include "filesystem_utils.h"
#include "logger.h"

cvar_t displaysoundlist = {"displaysoundlist", "0"};

// multiplayer server rules
cvar_t fragsleft = {"mp_fragsleft", "0", FCVAR_SERVER | FCVAR_UNLOGGED}; // Don't spam console/log files/users with this changing
cvar_t timeleft = {"mp_timeleft", "0", FCVAR_SERVER | FCVAR_UNLOGGED};	 // "      "

// multiplayer server rules
cvar_t teamplay = {"mp_teamplay", "0", FCVAR_SERVER};
cvar_t fraglimit = {"mp_fraglimit", "0", FCVAR_SERVER};
cvar_t timelimit = {"mp_timelimit", "0", FCVAR_SERVER};
cvar_t friendlyfire = {"mp_friendlyfire", "0", FCVAR_SERVER};
cvar_t falldamage = {"mp_falldamage", "0", FCVAR_SERVER};
cvar_t weaponstay = {"mp_weaponstay", "0", FCVAR_SERVER};
cvar_t forcerespawn = {"mp_forcerespawn", "1", FCVAR_SERVER};
cvar_t flashlight = {"mp_flashlight", "0", FCVAR_SERVER};
cvar_t aimcrosshair = {"mp_autocrosshair", "1", FCVAR_SERVER};
cvar_t decalfrequency = {"decalfrequency", "30", FCVAR_SERVER};
cvar_t teamlist = {"mp_teamlist", "hgrunt;scientist", FCVAR_SERVER};
cvar_t teamoverride = {"mp_teamoverride", "1"};
cvar_t defaultteam = {"mp_defaultteam", "0"};
cvar_t allowmonsters = {"mp_allowmonsters", "0", FCVAR_SERVER};

cvar_t allow_spectators = {"allow_spectators", "0.0", FCVAR_SERVER}; // 0 prevents players from being spectators

cvar_t mp_chattime = {"mp_chattime", "10", FCVAR_SERVER};

cvar_t sv_allowbunnyhopping = {"sv_allowbunnyhopping", "0", FCVAR_SERVER};

// LRC - SoHL cvars
cvar_t impulsetarget = {"sohl_impulsetarget", "0", FCVAR_SERVER};
cvar_t mw_debug = {"sohl_mw_debug", "0", FCVAR_SERVER};

// System-wide logger level: 0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR, 4=CRITICAL (default: 2)
cvar_t sohl_log_level = {"sohl_log_level", "2", FCVAR_SERVER};

//Macros to make skill cvars easier to define
#define DECLARE_SKILL_CVARS(name)                 \
	cvar_t sk_##name##1 = {"sk_" #name "1", "0"}; \
	cvar_t sk_##name##2 = {"sk_" #name "2", "0"}; \
	cvar_t sk_##name##3 = {"sk_" #name "3", "0"}

#define REGISTER_SKILL_CVARS(name) \
	CVAR_REGISTER(&sk_##name##1);  \
	CVAR_REGISTER(&sk_##name##2);  \
	CVAR_REGISTER(&sk_##name##3)

//CVARS FOR SKILL LEVEL SETTINGS
// Agrunt
DECLARE_SKILL_CVARS(agrunt_health);

DECLARE_SKILL_CVARS(agrunt_dmg_punch);

// Apache
DECLARE_SKILL_CVARS(apache_health);

// Barney
DECLARE_SKILL_CVARS(barney_health);

// Otis
DECLARE_SKILL_CVARS(otis_health);

// Bullsquid
DECLARE_SKILL_CVARS(bullsquid_health);

DECLARE_SKILL_CVARS(bullsquid_dmg_bite);

DECLARE_SKILL_CVARS(bullsquid_dmg_whip);

DECLARE_SKILL_CVARS(bullsquid_dmg_spit);

// Pit Drone
DECLARE_SKILL_CVARS(pitdrone_health);
DECLARE_SKILL_CVARS(pitdrone_dmg_bite);
DECLARE_SKILL_CVARS(pitdrone_dmg_whip);
DECLARE_SKILL_CVARS(pitdrone_dmg_spit);

// Big Momma
cvar_t sk_bigmomma_health_factor1 = {"sk_bigmomma_health_factor1", "1.0"};
cvar_t sk_bigmomma_health_factor2 = {"sk_bigmomma_health_factor2", "1.0"};
cvar_t sk_bigmomma_health_factor3 = {"sk_bigmomma_health_factor3", "1.0"};

cvar_t sk_bigmomma_dmg_slash1 = {"sk_bigmomma_dmg_slash1", "50"};
cvar_t sk_bigmomma_dmg_slash2 = {"sk_bigmomma_dmg_slash2", "50"};
cvar_t sk_bigmomma_dmg_slash3 = {"sk_bigmomma_dmg_slash3", "50"};

cvar_t sk_bigmomma_dmg_blast1 = {"sk_bigmomma_dmg_blast1", "100"};
cvar_t sk_bigmomma_dmg_blast2 = {"sk_bigmomma_dmg_blast2", "100"};
cvar_t sk_bigmomma_dmg_blast3 = {"sk_bigmomma_dmg_blast3", "100"};

cvar_t sk_bigmomma_radius_blast1 = {"sk_bigmomma_radius_blast1", "250"};
cvar_t sk_bigmomma_radius_blast2 = {"sk_bigmomma_radius_blast2", "250"};
cvar_t sk_bigmomma_radius_blast3 = {"sk_bigmomma_radius_blast3", "250"};

// Gargantua
DECLARE_SKILL_CVARS(gargantua_health);

DECLARE_SKILL_CVARS(gargantua_dmg_slash);

DECLARE_SKILL_CVARS(gargantua_dmg_fire);

DECLARE_SKILL_CVARS(gargantua_dmg_stomp);


// Hassassin
DECLARE_SKILL_CVARS(hassassin_health);


// Headcrab
DECLARE_SKILL_CVARS(headcrab_health);

DECLARE_SKILL_CVARS(headcrab_dmg_bite);

// Shock Roach
DECLARE_SKILL_CVARS(shockroach_health);
DECLARE_SKILL_CVARS(shockroach_dmg_bite);
DECLARE_SKILL_CVARS(shockroach_lifespan);

// Hgrunt
DECLARE_SKILL_CVARS(hgrunt_health);

DECLARE_SKILL_CVARS(hgrunt_kick);

DECLARE_SKILL_CVARS(hgrunt_pellets);

DECLARE_SKILL_CVARS(hgrunt_gspeed);

// Hgrunt Ally
DECLARE_SKILL_CVARS(hgrunt_ally_health);
DECLARE_SKILL_CVARS(hgrunt_ally_kick);
DECLARE_SKILL_CVARS(hgrunt_ally_pellets);
DECLARE_SKILL_CVARS(hgrunt_ally_gspeed);

// Hgrunt Medic
DECLARE_SKILL_CVARS(medic_ally_health);
DECLARE_SKILL_CVARS(medic_ally_kick);
DECLARE_SKILL_CVARS(medic_ally_pellets);
DECLARE_SKILL_CVARS(medic_ally_gspeed);
DECLARE_SKILL_CVARS(medic_ally_heal);

// Hgrunt Torch
DECLARE_SKILL_CVARS(torch_ally_health);
DECLARE_SKILL_CVARS(torch_ally_kick);
DECLARE_SKILL_CVARS(torch_ally_pellets);
DECLARE_SKILL_CVARS(torch_ally_gspeed);

// Male Assassin
DECLARE_SKILL_CVARS(massassin_health);
DECLARE_SKILL_CVARS(massassin_kick);
DECLARE_SKILL_CVARS(massassin_pellets);
DECLARE_SKILL_CVARS(massassin_gspeed);

// Shock Trooper
DECLARE_SKILL_CVARS(shocktrooper_health);
DECLARE_SKILL_CVARS(shocktrooper_kick);
DECLARE_SKILL_CVARS(shocktrooper_gspeed);
DECLARE_SKILL_CVARS(shocktrooper_maxcharge);
DECLARE_SKILL_CVARS(shocktrooper_rchgspeed);

// Houndeye
DECLARE_SKILL_CVARS(houndeye_health);

DECLARE_SKILL_CVARS(houndeye_dmg_blast);


// ISlave
DECLARE_SKILL_CVARS(islave_health);

DECLARE_SKILL_CVARS(islave_dmg_claw);

DECLARE_SKILL_CVARS(islave_dmg_clawrake);

DECLARE_SKILL_CVARS(islave_dmg_zap);


// Icthyosaur
DECLARE_SKILL_CVARS(ichthyosaur_health);

DECLARE_SKILL_CVARS(ichthyosaur_shake);


// Leech
DECLARE_SKILL_CVARS(leech_health);

DECLARE_SKILL_CVARS(leech_dmg_bite);

// Controller
DECLARE_SKILL_CVARS(controller_health);

DECLARE_SKILL_CVARS(controller_dmgzap);

DECLARE_SKILL_CVARS(controller_speedball);

DECLARE_SKILL_CVARS(controller_dmgball);

// Nihilanth
DECLARE_SKILL_CVARS(nihilanth_health);

DECLARE_SKILL_CVARS(nihilanth_zap);

// Scientist
DECLARE_SKILL_CVARS(scientist_health);

// Cleansuit Scientist
DECLARE_SKILL_CVARS(cleansuit_scientist_health);

// Snark
DECLARE_SKILL_CVARS(snark_health);

DECLARE_SKILL_CVARS(snark_dmg_bite);

DECLARE_SKILL_CVARS(snark_dmg_pop);

// Voltigore
DECLARE_SKILL_CVARS(voltigore_health);
DECLARE_SKILL_CVARS(voltigore_dmg_punch);
DECLARE_SKILL_CVARS(voltigore_dmg_beam);

// Baby Voltigore
DECLARE_SKILL_CVARS(babyvoltigore_health);
DECLARE_SKILL_CVARS(babyvoltigore_dmg_punch);

// Pit Worm
DECLARE_SKILL_CVARS(pitworm_health);
DECLARE_SKILL_CVARS(pitworm_dmg_swipe);
DECLARE_SKILL_CVARS(pitworm_dmg_beam);

// Gene Worm
DECLARE_SKILL_CVARS(geneworm_health);
DECLARE_SKILL_CVARS(geneworm_dmg_spit);
DECLARE_SKILL_CVARS(geneworm_dmg_hit);

// Zombie
DECLARE_SKILL_CVARS(zombie_health);

DECLARE_SKILL_CVARS(zombie_dmg_one_slash);

DECLARE_SKILL_CVARS(zombie_dmg_both_slash);

// Zombie Barney
DECLARE_SKILL_CVARS(zombie_barney_health);
DECLARE_SKILL_CVARS(zombie_barney_dmg_one_slash);
DECLARE_SKILL_CVARS(zombie_barney_dmg_both_slash);

// Zombie Soldier
DECLARE_SKILL_CVARS(zombie_soldier_health);
DECLARE_SKILL_CVARS(zombie_soldier_dmg_one_slash);
DECLARE_SKILL_CVARS(zombie_soldier_dmg_both_slash);

// Gonome
DECLARE_SKILL_CVARS(gonome_dmg_guts);
DECLARE_SKILL_CVARS(gonome_health);
DECLARE_SKILL_CVARS(gonome_dmg_one_slash);
DECLARE_SKILL_CVARS(gonome_dmg_one_bite);

//Turret
DECLARE_SKILL_CVARS(turret_health);


// MiniTurret
DECLARE_SKILL_CVARS(miniturret_health);


// Sentry Turret
DECLARE_SKILL_CVARS(sentry_health);


// PLAYER WEAPONS

// Crowbar whack
DECLARE_SKILL_CVARS(plr_crowbar);

// Glock Round
DECLARE_SKILL_CVARS(plr_9mm_bullet);

// 357 Round
DECLARE_SKILL_CVARS(plr_357_bullet);

// MP5 Round
DECLARE_SKILL_CVARS(plr_9mmAR_bullet);


// M203 grenade
DECLARE_SKILL_CVARS(plr_9mmAR_grenade);


// Shotgun buckshot
DECLARE_SKILL_CVARS(plr_buckshot);


// Crossbow
DECLARE_SKILL_CVARS(plr_xbow_bolt_client);

DECLARE_SKILL_CVARS(plr_xbow_bolt_monster);


// RPG
DECLARE_SKILL_CVARS(plr_rpg);


// Zero Point Generator
DECLARE_SKILL_CVARS(plr_gauss);


// Tau Cannon
DECLARE_SKILL_CVARS(plr_egon_narrow);

DECLARE_SKILL_CVARS(plr_egon_wide);


// Hand Grendade
DECLARE_SKILL_CVARS(plr_hand_grenade);


// Satchel Charge
DECLARE_SKILL_CVARS(plr_satchel);


// Tripmine
DECLARE_SKILL_CVARS(plr_tripmine);

// HORNET
DECLARE_SKILL_CVARS(plr_hornet_dmg);
// Pipe Wrench
DECLARE_SKILL_CVARS(plr_pipewrench);

// Knife
DECLARE_SKILL_CVARS(plr_knife);

// Grapple
DECLARE_SKILL_CVARS(plr_grapple);

// Desert Eagle
DECLARE_SKILL_CVARS(plr_eagle);

// Sniper Rifle
DECLARE_SKILL_CVARS(plr_762_bullet);

// M249
DECLARE_SKILL_CVARS(plr_556_bullet);

// Displacer
DECLARE_SKILL_CVARS(plr_displacer_self);
DECLARE_SKILL_CVARS(plr_displacer_other);
DECLARE_SKILL_CVARS(plr_displacer_radius);

// Shock Roach
DECLARE_SKILL_CVARS(plr_shockroachs);
DECLARE_SKILL_CVARS(plr_shockroachm);

// Spore Launcher
DECLARE_SKILL_CVARS(plr_spore);

// WORLD WEAPONS
DECLARE_SKILL_CVARS(12mm_bullet);

DECLARE_SKILL_CVARS(9mmAR_bullet);

DECLARE_SKILL_CVARS(9mm_bullet);


// HORNET
DECLARE_SKILL_CVARS(hornet_dmg);

// HEALTH/CHARGE
DECLARE_SKILL_CVARS(suitcharger);

DECLARE_SKILL_CVARS(battery);

DECLARE_SKILL_CVARS(healthcharger);

DECLARE_SKILL_CVARS(healthkit);

DECLARE_SKILL_CVARS(scientist_heal);

DECLARE_SKILL_CVARS(cleansuit_scientist_heal);


// monster damage adjusters
cvar_t sk_monster_head1 = {"sk_monster_head1", "2"};
cvar_t sk_monster_head2 = {"sk_monster_head2", "2"};
cvar_t sk_monster_head3 = {"sk_monster_head3", "2"};

cvar_t sk_monster_chest1 = {"sk_monster_chest1", "1"};
cvar_t sk_monster_chest2 = {"sk_monster_chest2", "1"};
cvar_t sk_monster_chest3 = {"sk_monster_chest3", "1"};

cvar_t sk_monster_stomach1 = {"sk_monster_stomach1", "1"};
cvar_t sk_monster_stomach2 = {"sk_monster_stomach2", "1"};
cvar_t sk_monster_stomach3 = {"sk_monster_stomach3", "1"};

cvar_t sk_monster_arm1 = {"sk_monster_arm1", "1"};
cvar_t sk_monster_arm2 = {"sk_monster_arm2", "1"};
cvar_t sk_monster_arm3 = {"sk_monster_arm3", "1"};

cvar_t sk_monster_leg1 = {"sk_monster_leg1", "1"};
cvar_t sk_monster_leg2 = {"sk_monster_leg2", "1"};
cvar_t sk_monster_leg3 = {"sk_monster_leg3", "1"};

// player damage adjusters
cvar_t sk_player_head1 = {"sk_player_head1", "2"};
cvar_t sk_player_head2 = {"sk_player_head2", "2"};
cvar_t sk_player_head3 = {"sk_player_head3", "2"};

cvar_t sk_player_chest1 = {"sk_player_chest1", "1"};
cvar_t sk_player_chest2 = {"sk_player_chest2", "1"};
cvar_t sk_player_chest3 = {"sk_player_chest3", "1"};

cvar_t sk_player_stomach1 = {"sk_player_stomach1", "1"};
cvar_t sk_player_stomach2 = {"sk_player_stomach2", "1"};
cvar_t sk_player_stomach3 = {"sk_player_stomach3", "1"};

cvar_t sk_player_arm1 = {"sk_player_arm1", "1"};
cvar_t sk_player_arm2 = {"sk_player_arm2", "1"};
cvar_t sk_player_arm3 = {"sk_player_arm3", "1"};

cvar_t sk_player_leg1 = {"sk_player_leg1", "1"};
cvar_t sk_player_leg2 = {"sk_player_leg2", "1"};
cvar_t sk_player_leg3 = {"sk_player_leg3", "1"};

// END Cvars for Skill Level settings

cvar_t sv_pushable_fixed_tick_fudge = {"sv_pushable_fixed_tick_fudge", "15"};

// BEGIN Opposing Force variables

cvar_t ctfplay = {"mp_ctfplay", "0", FCVAR_SERVER};
cvar_t ctf_autoteam = {"mp_ctf_autoteam", "0", FCVAR_SERVER};
cvar_t ctf_capture = {"mp_ctf_capture", "0", FCVAR_SERVER};

cvar_t coopplay = {"mp_coopplay", "0", FCVAR_SERVER};
cvar_t defaultcoop = {"mp_defaultcoop", "0", FCVAR_SERVER};
cvar_t coopweprespawn = {"mp_coopweprespawn", "0", FCVAR_SERVER};

cvar_t oldgrapple = {"sv_oldgrapple", "0", FCVAR_SERVER};
cvar_t oldweapons = {"sv_oldweapons", "0", FCVAR_SERVER};

cvar_t spamdelay = {"sv_spamdelay", "3.0", FCVAR_SERVER};
cvar_t multipower = {"mp_multipower", "0", FCVAR_SERVER};
cvar_t dmjumpsound = {"sv_dmjumpsound", "1", FCVAR_SERVER};

// END Opposing Force variables

static bool SV_InitServer()
{
	if (!FileSystem_LoadFileSystem())
	{
		return false;
	}

	if (UTIL_IsValveGameDirectory())
	{
		g_engfuncs.pfnServerPrint("This mod has detected that it is being run from a Valve game directory which is not supported\n"
			"Run this mod from its intended location\n\nThe game will now shut down\n");
		return false;
	}

	return true;
}

// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit()
{
	// Register cvars here:

	g_psv_gravity = CVAR_GET_POINTER("sv_gravity");
	g_psv_aim = CVAR_GET_POINTER("sv_aim");
	g_psv_allow_autoaim = CVAR_GET_POINTER("sv_allow_autoaim");
	g_footsteps = CVAR_GET_POINTER("mp_footsteps");
	g_psv_cheats = CVAR_GET_POINTER("sv_cheats");

	if (!SV_InitServer())
	{
		g_engfuncs.pfnServerPrint("Error initializing server\n");
		//Shut the game down as soon as possible.
		SERVER_COMMAND("quit\n");
		return;
	}

	CVAR_REGISTER(&displaysoundlist);
	CVAR_REGISTER(&allow_spectators);

	CVAR_REGISTER(&teamplay);
	CVAR_REGISTER(&fraglimit);
	CVAR_REGISTER(&timelimit);

	CVAR_REGISTER(&fragsleft);
	CVAR_REGISTER(&timeleft);

	CVAR_REGISTER(&friendlyfire);
	CVAR_REGISTER(&falldamage);
	CVAR_REGISTER(&weaponstay);
	CVAR_REGISTER(&forcerespawn);
	CVAR_REGISTER(&flashlight);
	CVAR_REGISTER(&aimcrosshair);
	CVAR_REGISTER(&decalfrequency);
	CVAR_REGISTER(&teamlist);
	CVAR_REGISTER(&teamoverride);
	CVAR_REGISTER(&defaultteam);
	CVAR_REGISTER(&allowmonsters);

	CVAR_REGISTER(&mp_chattime);

	CVAR_REGISTER(&sv_allowbunnyhopping);

	// LRC - SoHL cvars
	CVAR_REGISTER(&impulsetarget);
	CVAR_REGISTER(&mw_debug);
	CVAR_REGISTER(&sohl_log_level);
	g_Logger.Init(&sohl_log_level);

	// REGISTER CVARS FOR SKILL LEVEL STUFF
	// Agrunt
	REGISTER_SKILL_CVARS(agrunt_health);

	REGISTER_SKILL_CVARS(agrunt_dmg_punch);

	// Apache
	REGISTER_SKILL_CVARS(apache_health);

	// Barney
	REGISTER_SKILL_CVARS(barney_health);

	// Otis
	REGISTER_SKILL_CVARS(otis_health);

	// Bullsquid
	REGISTER_SKILL_CVARS(bullsquid_health);

	REGISTER_SKILL_CVARS(bullsquid_dmg_bite);

	REGISTER_SKILL_CVARS(bullsquid_dmg_whip);

	REGISTER_SKILL_CVARS(bullsquid_dmg_spit);

	// Pit Drone
	REGISTER_SKILL_CVARS(pitdrone_health);
	REGISTER_SKILL_CVARS(pitdrone_dmg_bite);
	REGISTER_SKILL_CVARS(pitdrone_dmg_whip);
	REGISTER_SKILL_CVARS(pitdrone_dmg_spit);

	REGISTER_SKILL_CVARS(bigmomma_health_factor);

	REGISTER_SKILL_CVARS(bigmomma_dmg_slash);

	REGISTER_SKILL_CVARS(bigmomma_dmg_blast);

	REGISTER_SKILL_CVARS(bigmomma_radius_blast);

	// Gargantua
	REGISTER_SKILL_CVARS(gargantua_health);

	REGISTER_SKILL_CVARS(gargantua_dmg_slash);

	REGISTER_SKILL_CVARS(gargantua_dmg_fire);

	REGISTER_SKILL_CVARS(gargantua_dmg_stomp);


	// Hassassin
	REGISTER_SKILL_CVARS(hassassin_health);


	// Headcrab
	REGISTER_SKILL_CVARS(headcrab_health);

	REGISTER_SKILL_CVARS(headcrab_dmg_bite);

	// Shock Roach
	REGISTER_SKILL_CVARS(shockroach_health);
	REGISTER_SKILL_CVARS(shockroach_dmg_bite);
	REGISTER_SKILL_CVARS(shockroach_lifespan);

	// Hgrunt
	REGISTER_SKILL_CVARS(hgrunt_health);

	REGISTER_SKILL_CVARS(hgrunt_kick);

	REGISTER_SKILL_CVARS(hgrunt_pellets);

	REGISTER_SKILL_CVARS(hgrunt_gspeed);

	// Hgrunt Ally
	REGISTER_SKILL_CVARS(hgrunt_ally_health);
	REGISTER_SKILL_CVARS(hgrunt_ally_kick);
	REGISTER_SKILL_CVARS(hgrunt_ally_pellets);
	REGISTER_SKILL_CVARS(hgrunt_ally_gspeed);

	// Hgrunt Medic
	REGISTER_SKILL_CVARS(medic_ally_health);
	REGISTER_SKILL_CVARS(medic_ally_kick);
	REGISTER_SKILL_CVARS(medic_ally_pellets);
	REGISTER_SKILL_CVARS(medic_ally_gspeed);
	REGISTER_SKILL_CVARS(medic_ally_heal);

	// Hgrunt Torch
	REGISTER_SKILL_CVARS(torch_ally_health);
	REGISTER_SKILL_CVARS(torch_ally_kick);
	REGISTER_SKILL_CVARS(torch_ally_pellets);
	REGISTER_SKILL_CVARS(torch_ally_gspeed);

	// Male Assassin
	REGISTER_SKILL_CVARS(massassin_health);
	REGISTER_SKILL_CVARS(massassin_kick);
	REGISTER_SKILL_CVARS(massassin_pellets);
	REGISTER_SKILL_CVARS(massassin_gspeed);

	// Shock Trooper
	REGISTER_SKILL_CVARS(shocktrooper_health);
	REGISTER_SKILL_CVARS(shocktrooper_kick);
	REGISTER_SKILL_CVARS(shocktrooper_gspeed);
	REGISTER_SKILL_CVARS(shocktrooper_maxcharge);
	REGISTER_SKILL_CVARS(shocktrooper_rchgspeed);

	// Houndeye
	REGISTER_SKILL_CVARS(houndeye_health);

	REGISTER_SKILL_CVARS(houndeye_dmg_blast);


	// ISlave
	REGISTER_SKILL_CVARS(islave_health);

	REGISTER_SKILL_CVARS(islave_dmg_claw);

	REGISTER_SKILL_CVARS(islave_dmg_clawrake);

	REGISTER_SKILL_CVARS(islave_dmg_zap);


	// Icthyosaur
	REGISTER_SKILL_CVARS(ichthyosaur_health);

	REGISTER_SKILL_CVARS(ichthyosaur_shake);



	// Leech
	REGISTER_SKILL_CVARS(leech_health);

	REGISTER_SKILL_CVARS(leech_dmg_bite);


	// Controller
	REGISTER_SKILL_CVARS(controller_health);

	REGISTER_SKILL_CVARS(controller_dmgzap);

	REGISTER_SKILL_CVARS(controller_speedball);

	REGISTER_SKILL_CVARS(controller_dmgball);

	// Nihilanth
	REGISTER_SKILL_CVARS(nihilanth_health);

	REGISTER_SKILL_CVARS(nihilanth_zap);

	// Scientist
	REGISTER_SKILL_CVARS(scientist_health);

	// Cleansuit Scientist
	REGISTER_SKILL_CVARS(cleansuit_scientist_health);

	// Snark
	REGISTER_SKILL_CVARS(snark_health);

	REGISTER_SKILL_CVARS(snark_dmg_bite);

	REGISTER_SKILL_CVARS(snark_dmg_pop);

	// Voltigore
	REGISTER_SKILL_CVARS(voltigore_health);
	REGISTER_SKILL_CVARS(voltigore_dmg_punch);
	REGISTER_SKILL_CVARS(voltigore_dmg_beam);

	// Baby Voltigore
	REGISTER_SKILL_CVARS(babyvoltigore_health);
	REGISTER_SKILL_CVARS(babyvoltigore_dmg_punch);

	// Pit Worm
	REGISTER_SKILL_CVARS(pitworm_health);
	REGISTER_SKILL_CVARS(pitworm_dmg_swipe);
	REGISTER_SKILL_CVARS(pitworm_dmg_beam);

	// Gene Worm
	REGISTER_SKILL_CVARS(geneworm_health);
	REGISTER_SKILL_CVARS(geneworm_dmg_spit);
	REGISTER_SKILL_CVARS(geneworm_dmg_hit);

	// Zombie
	REGISTER_SKILL_CVARS(zombie_health);

	REGISTER_SKILL_CVARS(zombie_dmg_one_slash);

	REGISTER_SKILL_CVARS(zombie_dmg_both_slash);

	// Zombie Barney
	REGISTER_SKILL_CVARS(zombie_barney_health);
	REGISTER_SKILL_CVARS(zombie_barney_dmg_one_slash);
	REGISTER_SKILL_CVARS(zombie_barney_dmg_both_slash);

	// Zombie Soldier
	REGISTER_SKILL_CVARS(zombie_soldier_health);
	REGISTER_SKILL_CVARS(zombie_soldier_dmg_one_slash);
	REGISTER_SKILL_CVARS(zombie_soldier_dmg_both_slash);

	// Gonome
	REGISTER_SKILL_CVARS(gonome_dmg_guts);
	REGISTER_SKILL_CVARS(gonome_health);
	REGISTER_SKILL_CVARS(gonome_dmg_one_slash);
	REGISTER_SKILL_CVARS(gonome_dmg_one_bite);


	//Turret
	REGISTER_SKILL_CVARS(turret_health);


	// MiniTurret
	REGISTER_SKILL_CVARS(miniturret_health);


	// Sentry Turret
	REGISTER_SKILL_CVARS(sentry_health);


	// PLAYER WEAPONS

	// Crowbar whack
	REGISTER_SKILL_CVARS(plr_crowbar);

	// Glock Round
	REGISTER_SKILL_CVARS(plr_9mm_bullet);

	// 357 Round
	REGISTER_SKILL_CVARS(plr_357_bullet);

	// MP5 Round
	REGISTER_SKILL_CVARS(plr_9mmAR_bullet);


	// M203 grenade
	REGISTER_SKILL_CVARS(plr_9mmAR_grenade);


	// Shotgun buckshot
	REGISTER_SKILL_CVARS(plr_buckshot);


	// Crossbow
	REGISTER_SKILL_CVARS(plr_xbow_bolt_monster);

	REGISTER_SKILL_CVARS(plr_xbow_bolt_client);


	// RPG
	REGISTER_SKILL_CVARS(plr_rpg);


	// Gauss Gun
	REGISTER_SKILL_CVARS(plr_gauss);


	// Egon Gun
	REGISTER_SKILL_CVARS(plr_egon_narrow);

	REGISTER_SKILL_CVARS(plr_egon_wide);


	// Hand Grendade
	REGISTER_SKILL_CVARS(plr_hand_grenade);


	// Satchel Charge
	REGISTER_SKILL_CVARS(plr_satchel);


	// Tripmine
	REGISTER_SKILL_CVARS(plr_tripmine);

	// HORNET
	REGISTER_SKILL_CVARS(plr_hornet_dmg);

	// Pipe Wrench
	REGISTER_SKILL_CVARS(plr_pipewrench);

	// Knife
	REGISTER_SKILL_CVARS(plr_knife);

	// Grapple
	REGISTER_SKILL_CVARS(plr_grapple);

	// Desert Eagle
	REGISTER_SKILL_CVARS(plr_eagle);

	// Sniper Rifle
	REGISTER_SKILL_CVARS(plr_762_bullet);

	// M249
	REGISTER_SKILL_CVARS(plr_556_bullet);

	// Displacer
	REGISTER_SKILL_CVARS(plr_displacer_self);
	REGISTER_SKILL_CVARS(plr_displacer_other);
	REGISTER_SKILL_CVARS(plr_displacer_radius);

	// Shock Roach
	REGISTER_SKILL_CVARS(plr_shockroachs);
	REGISTER_SKILL_CVARS(plr_shockroachm);

	// Spore Launcher
	REGISTER_SKILL_CVARS(plr_spore);


	// WORLD WEAPONS
	REGISTER_SKILL_CVARS(12mm_bullet);

	REGISTER_SKILL_CVARS(9mmAR_bullet);

	REGISTER_SKILL_CVARS(9mm_bullet);


	// HORNET
	REGISTER_SKILL_CVARS(hornet_dmg);

	// HEALTH/SUIT CHARGE DISTRIBUTION
	REGISTER_SKILL_CVARS(suitcharger);

	REGISTER_SKILL_CVARS(battery);

	REGISTER_SKILL_CVARS(healthcharger);

	REGISTER_SKILL_CVARS(healthkit);

	REGISTER_SKILL_CVARS(scientist_heal);

	REGISTER_SKILL_CVARS(cleansuit_scientist_heal);

	// monster damage adjusters
	REGISTER_SKILL_CVARS(monster_head);

	REGISTER_SKILL_CVARS(monster_chest);

	REGISTER_SKILL_CVARS(monster_stomach);

	REGISTER_SKILL_CVARS(monster_arm);

	REGISTER_SKILL_CVARS(monster_leg);

	// player damage adjusters
	REGISTER_SKILL_CVARS(player_head);

	REGISTER_SKILL_CVARS(player_chest);

	REGISTER_SKILL_CVARS(player_stomach);

	REGISTER_SKILL_CVARS(player_arm);

	REGISTER_SKILL_CVARS(player_leg);
	// END REGISTER CVARS FOR SKILL LEVEL STUFF

	CVAR_REGISTER(&sv_pushable_fixed_tick_fudge);

	// BEGIN REGISTER CVARS FOR OPPOSING FORCE

	CVAR_REGISTER(&ctfplay);
	CVAR_REGISTER(&ctf_autoteam);
	CVAR_REGISTER(&ctf_capture);

	CVAR_REGISTER(&coopplay);
	CVAR_REGISTER(&defaultcoop);
	CVAR_REGISTER(&coopweprespawn);

	CVAR_REGISTER(&oldgrapple);
	CVAR_REGISTER(&oldweapons);

	CVAR_REGISTER(&spamdelay);
	CVAR_REGISTER(&multipower);
	CVAR_REGISTER(&dmjumpsound);

	// END REGISTER CVARS FOR OPPOSING FORCE

	InitMapLoadingUtils();

	SERVER_COMMAND("exec skill.cfg\n");
	SERVER_COMMAND("exec skillopfor.cfg\n");
}

void GameDLLShutdown()
{
	g_Logger.Shutdown();
	FileSystem_FreeFileSystem();
}
