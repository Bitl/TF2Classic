//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Damage numers upon hitting an enemy
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "hud.h"
#include "hudelement.h"
#include "c_tf_player.h"
#include "view.h"
#include <vgui/IScheme.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

class CTFDamageAccountPanel : public EditablePanel, public CHudElement
{
private:
	DECLARE_CLASS_SIMPLE( CTFDamageAccountPanel, EditablePanel );

public:
	CTFDamageAccountPanel( const char *pElementName );

	virtual void LevelInit();
	virtual void Init();
	virtual void FireGameEvent( IGameEvent * event );
	virtual void ApplySchemeSettings( IScheme *pScheme );

	virtual bool ShouldDraw( void );
	virtual void PaintBackground( void );

private:
	Label	*m_pDamageAccountLabel;
	float	m_flRemoveAt; // Time to remove from view
};

// Register and set depth
DECLARE_HUDELEMENT_DEPTH(CTFDamageAccountPanel, 1);
// Create console var, to choose whether to show this or not
ConVar hud_combattext( "hud_combattext", "0", FCVAR_ARCHIVE, "");

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFDamageAccountPanel::CTFDamageAccountPanel( const char *pElementName )
	: EditablePanel( NULL, "HudDamageAccount" ), CHudElement ( pElementName )
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetScheme( "ClientScheme" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFDamageAccountPanel::LevelInit()
{
	SetVisible( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFDamageAccountPanel::Init()
{
	// listen for events
	ListenForGameEvent( "player_damaged" );

	SetVisible( false );
	CHudElement::Init( );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFDamageAccountPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/UI/HudDamageAccount.res" );

	m_pDamageAccountLabel = dynamic_cast< Label * >( FindChildByName( "CDamageAccountPanel" ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFDamageAccountPanel::FireGameEvent(IGameEvent * event)
{
	// Don't do anything if the user chooses not to view this
	if ( hud_combattext.GetBool() == false )
		return;

	const char *pEventName = event->GetName();

	if (Q_strcmp("player_damaged", pEventName) == 0)
	{
		if ( m_pDamageAccountLabel
			&& C_TFPlayer::GetLocalTFPlayer()->GetUserID() == event->GetInt( "userid_from" ) ) // Did we shoot the guy?
		{
			SetVisible( true );
			// Set remove time
			m_flRemoveAt = gpGlobals->curtime + 1.0f;
			// Set text to amount of damage
			char buffer[5]; // Up to four digits
			m_pDamageAccountLabel->SetText( itoa( event->GetInt( "amount" ), buffer, 10 ) );
			m_pDamageAccountLabel->SetVisible( true );

			// Respoition based on location of player hit
			int iX, iY;
			Vector vecTarget = Vector( event->GetFloat( "from_x" ), event->GetFloat( "from_y" ), event->GetFloat( "from_z" ) );
			bool bOnscreen = GetVectorInScreenSpace( vecTarget, iX, iY );
			int halfWidth = (GetWide() / 2) - 20; // A bit hacky
			if( bOnscreen )
				SetPos( iX - halfWidth, iY - ( GetTall() / 2 ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFDamageAccountPanel::ShouldDraw( void )
{
	return ( IsVisible( ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFDamageAccountPanel::PaintBackground( void )
{
	m_pDamageAccountLabel->SetFgColor( Color( 255, 0, 0, 255 ) );
	// Hide it?
	if( gpGlobals->curtime >= m_flRemoveAt )
	{
		SetAlpha( 0 ); // Using alphas for future fade-out effect
	}
	else
	{
		SetAlpha( 255 );
	}
}