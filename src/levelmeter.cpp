/******************************************************************************\
 * Copyright (c) 2004-2020
 *
 * Author(s):
 *  Volker Fischer
 *
 * Description:
 *  Implements a multi color LED bar
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
\******************************************************************************/

#include "levelmeter.h"


/* Implementation *************************************************************/
CLevelMeter::CLevelMeter ( QWidget* parent, Qt::WindowFlags f ) :
    QWidget ( parent, f ),
    eLevelMeterType ( MT_BAR )
{
    // initialize LED meter
    QWidget*     pLEDMeter  = new QWidget();
    QVBoxLayout* pLEDLayout = new QVBoxLayout ( pLEDMeter );
    pLEDLayout->setAlignment ( Qt::AlignHCenter );
    pLEDLayout->setMargin    ( 0 );
    pLEDLayout->setSpacing   ( 0 );

    // create LEDs
    vecpLEDs.Init ( NUM_STEPS_LED_BAR );

    for ( int iLEDIdx = NUM_STEPS_LED_BAR - 1; iLEDIdx >= 0; iLEDIdx-- )
    {
        // create LED object
        vecpLEDs[iLEDIdx] = new cLED ( parent );

        // add LED to layout with spacer (do not add spacer on the bottom of the first LED)
        if ( iLEDIdx < NUM_STEPS_LED_BAR - 1 )
        {
            pLEDLayout->addStretch();
        }

        pLEDLayout->addWidget ( vecpLEDs[iLEDIdx]->getLabelPointer() );
    }

    // initialize bar meter
    pBarMeter = new QProgressBar();
    pBarMeter->setOrientation ( Qt::Vertical );
    pBarMeter->setRange ( 0, 100 * NUM_STEPS_LED_BAR );
    pBarMeter->setFormat ( "" ); // suppress percent numbers

    // setup stacked layout for meter type switching mechanism
    pStackedLayout = new QStackedLayout ( this );
    pStackedLayout->addWidget ( pLEDMeter );
    pStackedLayout->addWidget ( pBarMeter );

    // according to QScrollArea description: "When using a scroll area to display the
    // contents of a custom widget, it is important to ensure that the size hint of
    // the child widget is set to a suitable value."
    pBarMeter->setMinimumSize ( QSize ( 1, 1 ) );
    pLEDMeter->setMinimumSize ( QSize ( 1, 1 ) );

    // update the meter type (using the default value of the meter type)
    SetLevelMeterType ( eLevelMeterType );
}

CLevelMeter::~CLevelMeter()
{
    // clean up the LED objects
    for ( int iLEDIdx = 0; iLEDIdx < NUM_STEPS_LED_BAR; iLEDIdx++ )
    {
        delete vecpLEDs[iLEDIdx];
    }
}

void CLevelMeter::changeEvent ( QEvent* curEvent )
{
    // act on enabled changed state
    if ( curEvent->type() == QEvent::EnabledChange )
    {
        // reset all LEDs
        Reset ( this->isEnabled() );
    }
}

void CLevelMeter::Reset ( const bool bEnabled )
{
    // update state of all LEDs
    for ( int iLEDIdx = 0; iLEDIdx < NUM_STEPS_LED_BAR; iLEDIdx++ )
    {
        // different reset behavoiur for enabled and disabled control
        if ( bEnabled )
        {
            vecpLEDs[iLEDIdx]->setColor ( cLED::RL_BLACK );
        }
        else
        {
            vecpLEDs[iLEDIdx]->setColor ( cLED::RL_DISABLED );
        }
    }
}

void CLevelMeter::SetLevelMeterType ( const ELevelMeterType eNType )
{
    eLevelMeterType = eNType;

    switch ( eNType )
    {
    case MT_LED:
        pStackedLayout->setCurrentIndex ( 0 );
        break;

    case MT_BAR:
        pStackedLayout->setCurrentIndex ( 1 );
        pBarMeter->setStyleSheet (
            "QProgressBar        { margin:     1px;"
            "                      padding:    1px; "
            "                      width:      15px; }"
            "QProgressBar::chunk { background: green; }" );
        break;

    case MT_SLIM_BAR:
        // set all LEDs to disabled, otherwise we would not get our desired small width
        for ( int iLEDIdx = 0; iLEDIdx < NUM_STEPS_LED_BAR; iLEDIdx++ )
        {
            vecpLEDs[iLEDIdx]->setColor ( cLED::RL_DISABLED );
        }

        pStackedLayout->setCurrentIndex ( 1 );
        pBarMeter->setStyleSheet (
            "QProgressBar        { border:     0px;"
            "                      margin:     0px;"
            "                      padding:    0px; "
            "                      width:      4px; }"
            "QProgressBar::chunk { background: green; }" );
        break;
    }
}

void CLevelMeter::SetValue ( const double dValue )
{
    if ( this->isEnabled() )
    {
        switch ( eLevelMeterType )
        {
        case MT_LED:
            // update state of all LEDs for current level value
            for ( int iLEDIdx = 0; iLEDIdx < NUM_STEPS_LED_BAR; iLEDIdx++ )
            {
                // set active LED color if value is above current LED index
                if ( iLEDIdx < dValue )
                {
                    // check which color we should use (green, yellow or red)
                    if ( iLEDIdx < YELLOW_BOUND_LED_BAR )
                    {
                        // green region
                        vecpLEDs[iLEDIdx]->setColor ( cLED::RL_GREEN );
                    }
                    else
                    {
                        if ( iLEDIdx < RED_BOUND_LED_BAR )
                        {
                            // yellow region
                            vecpLEDs[iLEDIdx]->setColor ( cLED::RL_YELLOW );
                        }
                        else
                        {
                            // red region
                            vecpLEDs[iLEDIdx]->setColor ( cLED::RL_RED );
                        }
                    }
                }
                else
                {
                    // we use grey LED for inactive state
                    vecpLEDs[iLEDIdx]->setColor ( cLED::RL_BLACK );
                }
            }
            break;

        case MT_BAR:
        case MT_SLIM_BAR:
            pBarMeter->setValue ( 100 * dValue );
            break;
        }
    }
}

CLevelMeter::cLED::cLED ( QWidget* parent ) :
    BitmCubeRoundBlack  ( QString::fromUtf8 ( ":/png/LEDs/res/HLEDBlackSmall.png" ) ),
    BitmCubeRoundGreen  ( QString::fromUtf8 ( ":/png/LEDs/res/HLEDGreenSmall.png" ) ),
    BitmCubeRoundYellow ( QString::fromUtf8 ( ":/png/LEDs/res/HLEDYellowSmall.png" ) ),
    BitmCubeRoundRed    ( QString::fromUtf8 ( ":/png/LEDs/res/HLEDRedSmall.png" ) )
{
    // create LED label
    pLEDLabel = new QLabel ( "", parent );

    // set initial bitmap
    pLEDLabel->setPixmap ( BitmCubeRoundBlack );
    eCurLightColor = RL_BLACK;
}

void CLevelMeter::cLED::setColor ( const ELightColor eNewColor )
{
    // only update LED if color has changed
    if ( eNewColor != eCurLightColor )
    {
        switch ( eNewColor )
        {
        case RL_DISABLED:
            // note that this is required for the compact channel mode
            pLEDLabel->setPixmap ( QPixmap() );
            break;

        case RL_BLACK:
            pLEDLabel->setPixmap ( BitmCubeRoundBlack );
            break;

        case RL_GREEN:
            pLEDLabel->setPixmap ( BitmCubeRoundGreen );
            break;

        case RL_YELLOW:
            pLEDLabel->setPixmap ( BitmCubeRoundYellow );
            break;

        case RL_RED:
            pLEDLabel->setPixmap ( BitmCubeRoundRed );
            break;
        }
        eCurLightColor = eNewColor;
    }
}
