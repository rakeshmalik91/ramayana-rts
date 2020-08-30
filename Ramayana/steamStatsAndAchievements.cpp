#include "stdafx.h"

#include "steamStatsAndAchievements.h"

#define _ACH_ID( id, name ) { id, #id, name, "", 0, 0 }

CSteamAchievements::CSteamAchievements(Achievement_t *Achievements, int NumAchievements) :
m_iAppID(0),
m_bInitialized(false),
m_CallbackUserStatsReceived(this, &CSteamAchievements::OnUserStatsReceived),
m_CallbackUserStatsStored(this, &CSteamAchievements::OnUserStatsStored),
m_CallbackAchievementStored(this, &CSteamAchievements::OnAchievementStored)
{
	m_iAppID = SteamUtils()->GetAppID();
	m_pAchievements = Achievements;
	m_iNumAchievements = NumAchievements;
	RequestStats();
}

bool CSteamAchievements::RequestStats()
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == SteamUserStats() || NULL == SteamUser())
	{
		return false;
	}
	// Is the user logged on?  If not we can't get stats.
	if (!SteamUser()->BLoggedOn())
	{
		return false;
	}
	// Request user stats.
	return SteamUserStats()->RequestCurrentStats();
}

bool CSteamAchievements::SetAchievement(const char* ID)
{
	// Have we received a call back from Steam yet?
	if (m_bInitialized)
	{
		SteamUserStats()->SetAchievement(ID);
		return SteamUserStats()->StoreStats();
	}
	// If not then we can't set achievements yet
	return false;
}

void CSteamAchievements::OnUserStatsReceived(UserStatsReceived_t *pCallback)
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (m_iAppID == pCallback->m_nGameID)
	{
		if (k_EResultOK == pCallback->m_eResult)
		{
			//showMessage("Received stats and achievements from Steam\n", "Steam");
			m_bInitialized = true;

			// load achievements
			for (int iAch = 0; iAch < m_iNumAchievements; ++iAch)
			{
				Achievement_t &ach = m_pAchievements[iAch];

				SteamUserStats()->GetAchievement(ach.m_pchAchievementID, &ach.m_bAchieved);
				_snprintf(ach.m_rgchName, sizeof(ach.m_rgchName), "%s",
					SteamUserStats()->GetAchievementDisplayAttribute(ach.m_pchAchievementID,
					"name"));
				_snprintf(ach.m_rgchDescription, sizeof(ach.m_rgchDescription), "%s",
					SteamUserStats()->GetAchievementDisplayAttribute(ach.m_pchAchievementID,
					"desc"));
			}
		} else
		{
			char buffer[128];
			_snprintf(buffer, 128, "RequestStats - failed, %d\n", pCallback->m_eResult);
			//showMessage(buffer, "Steam");
		}
	}
}


void CSteamAchievements::OnUserStatsStored(UserStatsStored_t *pCallback)
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (m_iAppID == pCallback->m_nGameID)
	{
		if (k_EResultOK == pCallback->m_eResult)
		{
			//showMessage("Stored stats for Steam\n", "Steam");
		} else
		{
			char buffer[128];
			_snprintf(buffer, 128, "StatsStored - failed, %d\n", pCallback->m_eResult);
			//showMessage(buffer, "Steam");
		}
	}
}


void CSteamAchievements::OnAchievementStored(UserAchievementStored_t *pCallback)
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (m_iAppID == pCallback->m_nGameID)
	{
		//showMessage("Stored Achievement for Steam\n", "Steam");
	}
}

// Defining our achievements
enum EAchievements
{
	ACH_CMPN_2 = 0,
	ACH_FIRE_IN_WATER = 1,
	ACH_GIANTS_DUEL = 2,
	ACH_CREATED_A_MAP = 3,
	ACH_CMPN_10 = 4,
	ACH_CMPN_12 = 5,
	ACH_USE_BRAHMASTRA = 6,
	ACH_CMPN_20 = 7,
	ACH_CMPN_17 = 8,
	ACH_CMPN_5 = 9,
};

// Achievement array which will hold data about the achievements and their state
Achievement_t g_Achievements[] =
{
	_ACH_ID(ACH_CMPN_2, "Graduated"),
	_ACH_ID(ACH_CMPN_5, "Engineer"),
	_ACH_ID(ACH_CMPN_10, "Giant-Slayer"),
	_ACH_ID(ACH_CMPN_12, "Trickster"),
	_ACH_ID(ACH_CMPN_17, "Heavyweight"),
	_ACH_ID(ACH_CMPN_20, "Demon-Slayer"),
	_ACH_ID(ACH_USE_BRAHMASTRA, "Brahmastra"),
	_ACH_ID(ACH_FIRE_IN_WATER, "Fire In Water"),
	_ACH_ID(ACH_GIANTS_DUEL, "Clash of the Titans"),
	_ACH_ID(ACH_CREATED_A_MAP, "Cartographer"),
};

CSteamAchievements* g_SteamAchievements = NULL;

void initSteam() {
	// Initialize Steam
	bool bRet = SteamAPI_Init();
	// Create the SteamAchievements object if Steam was successfully initialized
	if (bRet)
	{
		g_SteamAchievements = new CSteamAchievements(g_Achievements, 4);
	}
}

CSteamAchievements* getSteamAchievements() {
	return g_SteamAchievements;
}