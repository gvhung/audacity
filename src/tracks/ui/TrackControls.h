/**********************************************************************

Audacity: A Digital Audio Editor

TrackControls.h

Paul Licameli split from TrackPanel.cpp

**********************************************************************/

#ifndef __AUDACITY_TRACK_CONTROLS__
#define __AUDACITY_TRACK_CONTROLS__

#include "CommonTrackPanelCell.h"
#include "../../MemoryX.h"

class PopupMenuTable;
class Track;

class TrackControls /* not final */ : public CommonTrackPanelCell
{
public:
   explicit
   TrackControls( std::shared_ptr<Track> pTrack );

   virtual ~TrackControls() = 0;

   std::shared_ptr<Track> FindTrack() override;

   // This is passed to the InitMenu() methods of the PopupMenuTable
   // objects returned by GetMenuExtension:
   struct InitMenuData
   {
   public:
      Track *pTrack;
      wxWindow *pParent;
      unsigned result;
   };

   // Make this hack go away!  See TrackPanel::DrawOutside
   static int gCaptureState;

protected:
   // An override is supplied for derived classes to call through but it is
   // still marked pure virtual
   virtual HitTestResult HitTest
      (const TrackPanelMouseEvent &event,
       const AudacityProject *) override = 0;

   unsigned DoContextMenu
      (const wxRect &rect, wxWindow *pParent, wxPoint *pPosition) override;
   virtual PopupMenuTable *GetMenuExtension(Track *pTrack) = 0;

   Track *GetTrack() const;

   std::weak_ptr<Track> mwTrack;
};

#endif
