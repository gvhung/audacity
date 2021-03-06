/**********************************************************************

Audacity: A Digital Audio Editor

NoteTrackVRulerControls.cpp

Paul Licameli split from TrackPanel.cpp

**********************************************************************/

#include "../../../../Audacity.h"

#ifdef USE_MIDI

#include "NoteTrackVRulerControls.h"

#include "../../../../HitTestResult.h"
#include "../../../../NoteTrack.h"
#include "../../../../Project.h"
#include "../../../../RefreshCode.h"
#include "../../../../TrackPanelMouseEvent.h"
#include "../../../../UIHandle.h"
#include "../../../../../images/Cursors.h"

namespace
{

   bool IsDragZooming(int zoomStart, int zoomEnd)
   {
      const int DragThreshold = 3;// Anything over 3 pixels is a drag, else a click.
      return (abs(zoomEnd - zoomStart) > DragThreshold);
   }

}

///////////////////////////////////////////////////////////////////////////////

class NoteTrackVZoomHandle : public UIHandle
{
   NoteTrackVZoomHandle();
   NoteTrackVZoomHandle(const NoteTrackVZoomHandle&);
   NoteTrackVZoomHandle &operator=(const NoteTrackVZoomHandle&);
   static NoteTrackVZoomHandle& Instance();
   static HitTestPreview HitPreview(const wxMouseEvent &event);

public:
   static HitTestResult HitTest(const wxMouseEvent &event);

   virtual ~NoteTrackVZoomHandle();

   virtual Result Click
      (const TrackPanelMouseEvent &event, AudacityProject *pProject);

   virtual Result Drag
      (const TrackPanelMouseEvent &event, AudacityProject *pProject);

   virtual HitTestPreview Preview
      (const TrackPanelMouseEvent &event, const AudacityProject *pProject);

   virtual Result Release
      (const TrackPanelMouseEvent &event, AudacityProject *pProject,
       wxWindow *pParent);

   virtual Result Cancel(AudacityProject *pProject);

   virtual void DrawExtras
      (DrawingPass pass,
      wxDC * dc, const wxRegion &updateRegion, const wxRect &panelRect);

private:
   std::weak_ptr<NoteTrack> mpTrack;

   int mZoomStart, mZoomEnd;
   wxRect mRect;
};

NoteTrackVZoomHandle::NoteTrackVZoomHandle()
   : mZoomStart(0), mZoomEnd(0), mRect()
{
}

NoteTrackVZoomHandle &NoteTrackVZoomHandle::Instance()
{
   static NoteTrackVZoomHandle instance;
   return instance;
}

HitTestPreview NoteTrackVZoomHandle::HitPreview(const wxMouseEvent &event)
{
   static auto zoomInCursor =
      ::MakeCursor(wxCURSOR_MAGNIFIER, ZoomInCursorXpm, 19, 15);
   static auto zoomOutCursor =
      ::MakeCursor(wxCURSOR_MAGNIFIER, ZoomOutCursorXpm, 19, 15);
   return {
      _("Click to verticaly zoom in, Shift-click to zoom out, Drag to create a particular zoom region."),
      (event.ShiftDown() ? &*zoomOutCursor : &*zoomInCursor)
   };
}

HitTestResult NoteTrackVZoomHandle::HitTest(const wxMouseEvent &event)
{
   return HitTestResult(HitPreview(event), &Instance());
}

NoteTrackVZoomHandle::~NoteTrackVZoomHandle()
{
}

UIHandle::Result NoteTrackVZoomHandle::Click
(const TrackPanelMouseEvent &evt, AudacityProject *pProject)
{
   mpTrack = std::static_pointer_cast<NoteTrack>(
      static_cast<NoteTrackVRulerControls*>(evt.pCell.get())->FindTrack() );
   mRect = evt.rect;

   const wxMouseEvent &event = evt.event;
   mZoomStart = event.m_y;
   mZoomEnd = event.m_y;

   // change note track to zoom like audio track
   //          mpTrack->StartVScroll();

   return RefreshCode::RefreshNone;
}

UIHandle::Result NoteTrackVZoomHandle::Drag
(const TrackPanelMouseEvent &evt, AudacityProject *pProject)
{
   using namespace RefreshCode;
   auto pTrack = pProject->GetTracks()->Lock(mpTrack);
   if (!pTrack)
      return Cancelled;

   const wxMouseEvent &event = evt.event;
   mZoomEnd = event.m_y;
   if (IsDragZooming(mZoomStart, mZoomEnd)) {
      // changed Note track to work like audio track
      //         pTrack->VScroll(mZoomStart, mZoomEnd);
      return RefreshAll;
   }
   return RefreshNone;
}

HitTestPreview NoteTrackVZoomHandle::Preview
(const TrackPanelMouseEvent &evt, const AudacityProject *pProject)
{
   return HitPreview(evt.event);
}

UIHandle::Result NoteTrackVZoomHandle::Release
(const TrackPanelMouseEvent &evt, AudacityProject *pProject,
 wxWindow *pParent)
{
   using namespace RefreshCode;
   auto pTrack = pProject->GetTracks()->Lock(mpTrack);
   if (!pTrack)
      return RefreshNone;

   const wxMouseEvent &event = evt.event;
   if (IsDragZooming(mZoomStart, mZoomEnd)) {
      pTrack->ZoomTo(evt.rect, mZoomStart, mZoomEnd);
   }
   else if (event.ShiftDown() || event.RightUp()) {
      pTrack->ZoomOut(evt.rect, mZoomEnd);
   }
   else {
      pTrack->ZoomIn(evt.rect, mZoomEnd);
   }

   // TODO:  shift-right click as in audio track?

   mZoomEnd = mZoomStart = 0;
   pProject->ModifyState(true);

   return RefreshAll;
}

UIHandle::Result NoteTrackVZoomHandle::Cancel(AudacityProject *pProject)
{
   // Cancel is implemented!  And there is no initial state to restore,
   // so just return a code.
   return RefreshCode::RefreshAll;
}

void NoteTrackVZoomHandle::DrawExtras
(DrawingPass pass, wxDC * dc, const wxRegion &, const wxRect &panelRect)
{
   if (!mpTrack.lock()) //? TrackList::Lock()
      return;

   if ( pass == UIHandle::Cells &&
        IsDragZooming( mZoomStart, mZoomEnd ) )
      TrackVRulerControls::DrawZooming
         ( dc, mRect, panelRect, mZoomStart, mZoomEnd );
}

///////////////////////////////////////////////////////////////////////////////
NoteTrackVRulerControls::~NoteTrackVRulerControls()
{
}

HitTestResult NoteTrackVRulerControls::HitTest
(const TrackPanelMouseEvent &evt,
 const AudacityProject *)
{
#ifdef USE_MIDI
   return NoteTrackVZoomHandle::HitTest(evt.event);
#else
   return {};
#endif
}

#endif