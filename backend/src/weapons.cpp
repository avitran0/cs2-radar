#include "weapons.h"

#include <string>

Weapon string_to_weapon(std::string weapon_name) {
    if (weapon_name == "weapon_glock18") {
        return WEAPON_GLOCK;
    } else if (weapon_name == "weapon_hkp2000") {
        return WEAPON_P2000;
    } else if (weapon_name == "weapon_usp_silencer") {
        return WEAPON_USPS;
    } else if (weapon_name == "weapon_p250") {
        return WEAPON_P250;
    } else if (weapon_name == "weapon_tec9") {
        return WEAPON_TEC9;
    } else if (weapon_name == "weapon_cz75a") {
        return WEAPON_CZ75;
    } else if (weapon_name == "weapon_fiveseven") {
        return WEAPON_FIVESEVEN;
    } else if (weapon_name == "weapon_elite") {
        return WEAPON_DUAL_BERETTAS;
    } else if (weapon_name == "weapon_revolver") {
        return WEAPON_REVOLVER;
    } else if (weapon_name == "weapon_deagle") {
        return WEAPON_DEAGLE;
    } else if (weapon_name == "weapon_nova") {
        return WEAPON_NOVA;
    } else if (weapon_name == "weapon_xm1014") {
        return WEAPON_XM1014;
    } else if (weapon_name == "weapon_mag7") {
        return WEAPON_MAG7;
    } else if (weapon_name == "weapon_sawedoff") {
        return WEAPON_SAWED_OFF;
    } else if (weapon_name == "weapon_mac10") {
        return WEAPON_MAC10;
    } else if (weapon_name == "weapon_mp9") {
        return WEAPON_MP9;
    } else if (weapon_name == "weapon_mp7") {
        return WEAPON_MP7;
    } else if (weapon_name == "weapon_mp5sd") {
        return WEAPON_MP5;
    } else if (weapon_name == "weapon_ump45") {
        return WEAPON_UMP45;
    } else if (weapon_name == "weapon_p90") {
        return WEAPON_P90;
    } else if (weapon_name == "weapon_bizon") {
        return WEAPON_BIZON;
    } else if (weapon_name == "weapon_galilar") {
        return WEAPON_GALIL;
    } else if (weapon_name == "weapon_famas") {
        return WEAPON_FAMAS;
    } else if (weapon_name == "weapon_m4a1") {
        return WEAPON_M4A4;
    } else if (weapon_name == "weapon_m4a1_silencer") {
        return WEAPON_M4A1S;
    } else if (weapon_name == "weapon_ak47") {
        return WEAPON_AK47;
    } else if (weapon_name == "weapon_sg556") {
        return WEAPON_SG553;
    } else if (weapon_name == "weapon_aug") {
        return WEAPON_AUG;
    } else if (weapon_name == "weapon_scar20") {
        return WEAPON_SCAR20;
    } else if (weapon_name == "weapon_g3sg1") {
        return WEAPON_G3SG1;
    } else if (weapon_name == "weapon_awp") {
        return WEAPON_AWP;
    } else if (weapon_name == "weapon_ssg08") {
        return WEAPON_SSG08;
    } else if (weapon_name == "weapon_m249") {
        return WEAPON_M249;
    } else if (weapon_name == "weapon_negev") {
        return WEAPON_NEGEV;
    } else if (weapon_name == "weapon_knife") {
        return WEAPON_KNIFE;
    } else if (weapon_name == "weapon_knife_t") {
        return WEAPON_KNIFE_T;
    } else if (weapon_name == "weapon_flashbang") {
        return WEAPON_FLASHBANG;
    } else if (weapon_name == "weapon_hegrenade") {
        return WEAPON_HEGRENADE;
    } else if (weapon_name == "weapon_smokegrenade") {
        return WEAPON_SMOKEGRENADE;
    } else if (weapon_name == "weapon_molotov") {
        return WEAPON_MOLOTOV;
    } else if (weapon_name == "weapon_decoy") {
        return WEAPON_DECOY;
    } else if (weapon_name == "weapon_incgrenade") {
        return WEAPON_INCENDIARY;
    } else if (weapon_name == "weapon_c4") {
        return WEAPON_C4;
    } else if (weapon_name == "weapon_bayonet") {
        return WEAPON_KNIFE_BAYONET;
    } else if (weapon_name == "weapon_knife_survival_bowie") {
        return WEAPON_KNIFE_BOWIE;
    } else if (weapon_name == "weapon_knife_butterfly") {
        return WEAPON_KNIFE_BUTTERFLY;
    } else if (weapon_name == "weapon_knife_css") {
        return WEAPON_KNIFE_CLASSIC;
    } else if (weapon_name == "weapon_knife_falchion") {
        return WEAPON_KNIFE_FALCHION;
    } else if (weapon_name == "weapon_knife_flip") {
        return WEAPON_KNIFE_FLIP;
    } else if (weapon_name == "weapon_knife_gut") {
        return WEAPON_KNIFE_GUT;
    } else if (weapon_name == "weapon_knife_tactical") {
        return WEAPON_KNIFE_HUNTSMAN;
    } else if (weapon_name == "weapon_knife_karambit") {
        return WEAPON_KNIFE_KARAMBIT;
    } else if (weapon_name == "weapon_knife_kukri") {
        return WEAPON_KNIFE_KUKRI;
    } else if (weapon_name == "weapon_knife_m9_bayonet") {
        return WEAPON_KNIFE_M9_BAYONET;
    } else if (weapon_name == "weapon_knife_gypsy_jackknife") {
        return WEAPON_KNIFE_NAVAJA;
    } else if (weapon_name == "weapon_knife_outdoor") {
        return WEAPON_KNIFE_NOMAD;
    } else if (weapon_name == "weapon_knife_cord") {
        return WEAPON_KNIFE_PARACORD;
    } else if (weapon_name == "weapon_knife_push") {
        return WEAPON_KNIFE_SHADOW_DAGGERS;
    } else if (weapon_name == "weapon_knife_skeleton") {
        return WEAPON_KNIFE_SKELETON;
    } else if (weapon_name == "weapon_knife_stiletto") {
        return WEAPON_KNIFE_STILETTO;
    } else if (weapon_name == "weapon_knife_canis") {
        return WEAPON_KNIFE_SURVIVAL;
    } else if (weapon_name == "weapon_knife_widowmaker") {
        return WEAPON_KNIFE_TALON;
    } else if (weapon_name == "weapon_knife_ursus") {
        return WEAPON_KNIFE_URSUS;
    }

    return WEAPON_UNKNOWN;
}
