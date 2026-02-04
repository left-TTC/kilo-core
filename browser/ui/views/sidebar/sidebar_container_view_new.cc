/* Copyright (c) 2026 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/sidebar/sidebar_container_view_new.h"

#include <algorithm>

#include "base/check.h"
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/time/time.h"
#include "brave/browser/ui/brave_browser.h"
#include "brave/browser/ui/color/brave_color_id.h"
#include "brave/browser/ui/sidebar/sidebar_controller.h"
#include "brave/browser/ui/sidebar/sidebar_service_factory.h"
#include "brave/browser/ui/views/frame/brave_browser_view.h"
#include "brave/browser/ui/views/sidebar/sidebar_control_view.h"
#include "brave/browser/ui/views/toolbar/brave_toolbar_view.h"
#include "brave/browser/ui/views/toolbar/side_panel_button.h"
#include "brave/components/constants/pref_names.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/browser_window/public/browser_window_features.h"
#include "chrome/browser/ui/color/chrome_color_id.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/toolbar/toolbar_view.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/compositor/layer.h"
#include "ui/events/event_observer.h"
#include "ui/gfx/animation/tween.h"
#include "ui/gfx/geometry/point.h"
#include "ui/views/border.h"
#include "ui/views/event_monitor.h"
#include "ui/views/view_utils.h"
#include "ui/views/widget/widget.h"

namespace {

using ShowSidebarOption = sidebar::SidebarService::ShowSidebarOption;

sidebar::SidebarService* GetSidebarService(BraveBrowser* browser) {
  return sidebar::SidebarServiceFactory::GetForProfile(browser->profile());
}

}  // namespace

class SidebarContainerViewNew::BrowserWindowEventObserver
    : public ui::EventObserver {
 public:
  explicit BrowserWindowEventObserver(SidebarContainerViewNew& host)
      : host_(host) {}
  ~BrowserWindowEventObserver() override = default;
  BrowserWindowEventObserver(const BrowserWindowEventObserver&) = delete;
  BrowserWindowEventObserver& operator=(const BrowserWindowEventObserver&) =
      delete;

  void OnEvent(const ui::Event& event) override {
    DCHECK(event.IsMouseEvent());
    const auto* mouse_event = event.AsMouseEvent();

    gfx::Point window_event_position = mouse_event->location();
    // Convert window position to sidebar view's coordinate and check whether
    // it's included in sidebar ui or not.
    // If it's not included and sidebar could be hidden, stop monitoring and
    // hide UI.
    views::View::ConvertPointFromWidget(host_->sidebar_control_view_,
                                        &window_event_position);
    if (!host_->sidebar_control_view_->GetLocalBounds().Contains(
            window_event_position) &&
        !host_->ShouldForceShowSidebar()) {
      host_->StopBrowserWindowEventMonitoring();
      host_->HideSidebarControlView();
    }
  }

 private:
  const raw_ref<SidebarContainerViewNew> host_;
};

SidebarContainerViewNew::SidebarContainerViewNew(Browser* browser)
    : views::AnimationDelegateViews(this),
      browser_(browser),
      browser_window_event_observer_(
          std::make_unique<BrowserWindowEventObserver>(*this)) {
  constexpr int kAnimationDurationMS = 150;
  width_animation_.SetSlideDuration(base::Milliseconds(kAnimationDurationMS));

  SetNotifyEnterExitOnChild(true);
}

SidebarContainerViewNew::~SidebarContainerViewNew() = default;

void SidebarContainerViewNew::Init() {
  initialized_ = true;

  sidebar_model_ = browser_->GetFeatures().sidebar_controller()->model();
  sidebar_model_observation_.Observe(sidebar_model_);

  show_side_panel_button_.Init(
      kShowSidePanelButton, browser_->profile()->GetPrefs(),
      base::BindRepeating(
          &SidebarContainerViewNew::UpdateToolbarButtonVisibility,
          base::Unretained(this)));

  AddChildViews();
  UpdateToolbarButtonVisibility();
  SetSidebarShowOption(
      GetSidebarService(GetBraveBrowser())->GetSidebarShowOption());
}

void SidebarContainerViewNew::SetSidebarOnLeft(bool sidebar_on_left) {
  DCHECK(initialized_);

  if (sidebar_on_left_ == sidebar_on_left) {
    return;
  }

  sidebar_on_left_ = sidebar_on_left;

  DCHECK(sidebar_control_view_);
  sidebar_control_view_->SetSidebarOnLeft(sidebar_on_left_);
}

bool SidebarContainerViewNew::IsSidebarVisible() const {
  return sidebar_control_view_ && sidebar_control_view_->GetVisible();
}

void SidebarContainerViewNew::ShowSidebarOnMouseOver(
    const gfx::PointF& point_in_screen) {
  if (IsSidebarVisible()) {
    return;
  }

  if (show_sidebar_option_ != ShowSidebarOption::kShowOnMouseOver) {
    return;
  }

  gfx::RectF mouse_event_detect_bounds(
      BraveBrowserView::From(BrowserView::GetBrowserViewForBrowser(browser_))
          ->GetBoundingBoxInScreenForMouseOverHandling());

  constexpr int kHotCornerWidth = 7;
  const int inset = mouse_event_detect_bounds.width() - kHotCornerWidth;
  if (sidebar_on_left_) {
    mouse_event_detect_bounds.Inset(gfx::InsetsF::TLBR(0, 0, 0, inset));
  } else {
    mouse_event_detect_bounds.Inset(gfx::InsetsF::TLBR(0, inset, 0, 0));
  }

  if (!mouse_event_detect_bounds.Contains(point_in_screen)) {
    return;
  }

  ShowSidebarControlView();
}

void SidebarContainerViewNew::UpdateBorder() {
  sidebar_control_view_->UpdateBackgroundAndBorder();
}

void SidebarContainerViewNew::SetSidebarShowOption(
    ShowSidebarOption show_option) {
  show_sidebar_option_ = show_option;

  if (show_sidebar_option_ == ShowSidebarOption::kShowAlways) {
    ShowSidebarControlView();
    return;
  }

  if (show_sidebar_option_ == ShowSidebarOption::kShowNever) {
    HideSidebarControlView();
    return;
  }

  // kShowOnMouseOver
  if (IsMouseHovered()) {
    ShowSidebarControlView();
  } else {
    HideSidebarControlView();
  }
}

void SidebarContainerViewNew::UpdateSidebarItemsState() {
  // control view has items.
  sidebar_control_view_->Update();
}

void SidebarContainerViewNew::MenuClosed() {
  // Don't need to auto hide sidebar UI for other options.
  if (show_sidebar_option_ != ShowSidebarOption::kShowOnMouseOver) {
    return;
  }

  // Don't hide sidebar with below conditions.
  if (IsMouseHovered() || ShouldForceShowSidebar()) {
    return;
  }

  HideSidebarControlView();
}

void SidebarContainerViewNew::UpdateBackground() {
  if (const ui::ColorProvider* color_provider = GetColorProvider()) {
    SetBackground(
        views::CreateSolidBackground(color_provider->GetColor(kColorToolbar)));
  }
}

void SidebarContainerViewNew::AddChildViews() {
  sidebar_control_view_ = AddChildView(
      std::make_unique<SidebarControlView>(this, GetBraveBrowser()));
  sidebar_control_view_->SetPaintToLayer();

  // To prevent showing layered-children while its bounds is invisible.
  sidebar_control_view_->layer()->SetMasksToBounds(true);

  // Hide by default. Visibility will be controlled by show options callback
  // later.
  sidebar_control_view_->SetVisible(false);
}

void SidebarContainerViewNew::Layout(PassKey) {
  if (!initialized_) {
    LayoutSuperclass<views::View>(this);
    return;
  }

  // As control view uses its own layer, we should set its size exactly.
  // Otherwise, it's rendered even parent rect width is zero.
  int control_view_width =
      std::min(sidebar_control_view_->GetPreferredSize().width(), width());

  const int control_view_x =
      sidebar_on_left_ ? 0 : width() - control_view_width;

  sidebar_control_view_->SetBounds(control_view_x, 0, control_view_width,
                                   height());
}

gfx::Size SidebarContainerViewNew::CalculatePreferredSize(
    const views::SizeBounds& available_size) const {
  if (!initialized_ || !sidebar_control_view_->GetVisible()) {
    return View::CalculatePreferredSize(available_size);
  }

  auto start_width = animation_start_width_;
  auto end_width = animation_end_width_;
  if (width_animation_.IsClosing()) {
    start_width = animation_end_width_;
    end_width = animation_start_width_;
  }

  if (width_animation_.is_animating()) {
    return {gfx::Tween::IntValueBetween(width_animation_.GetCurrentValue(),
                                        start_width, end_width),
            0};
  }

  int preferred_width = 0;
  if (sidebar_control_view_->GetVisible()) {
    preferred_width = sidebar_control_view_->GetPreferredSize().width();
  }

  return {preferred_width, 0};
}

void SidebarContainerViewNew::OnThemeChanged() {
  View::OnThemeChanged();
  UpdateBackground();
  UpdateBorder();
}

void SidebarContainerViewNew::OnMouseEntered(const ui::MouseEvent& event) {
  // View::OnMouseEntered() is called when mouse moves from parent rect to
  // child views. But in case of control view we want to track if the mouse
  // has entered anywhere within control view bounds. This is because we want
  // to stop hide timer when mouse moves from one button to another.
  // We also want to show the sidebar when mouse enters the container bounds.

  if (show_sidebar_option_ != ShowSidebarOption::kShowOnMouseOver) {
    return;
  }

  sidebar_hide_timer_.Stop();

  if (!sidebar_control_view_->GetVisible()) {
    ShowSidebarControlView();
  }
}

void SidebarContainerViewNew::OnMouseExited(const ui::MouseEvent& event) {
  if (show_sidebar_option_ != ShowSidebarOption::kShowOnMouseOver) {
    return;
  }

  if (ShouldForceShowSidebar()) {
    StartBrowserWindowEventMonitoring();
    return;
  }

  // Hide w/ timer so that user can choose another button in control view.
  constexpr int kSidebarHideDelayInMS = 400;
  sidebar_hide_timer_.Start(
      FROM_HERE, base::Milliseconds(kSidebarHideDelayInMS),
      base::BindOnce(
          &SidebarContainerViewNew::HideSidebarControlViewForShowOption,
          base::Unretained(this)));
}

void SidebarContainerViewNew::AnimationProgressed(
    const gfx::Animation* animation) {
  DCHECK_EQ(animation, &width_animation_);

  // Relayout here because our preferred width is calculated based on animation
  // progress.
  if (GetWidget()) {
    GetWidget()->LayoutRootViewIfNecessary();
  }
}

void SidebarContainerViewNew::AnimationEnded(const gfx::Animation* animation) {
  DCHECK_EQ(animation, &width_animation_);

  if (!width_animation_.IsShowing()) {
    sidebar_control_view_->SetVisible(false);
  }

  // Relayout here so that final state is reflected.
  if (GetWidget()) {
    GetWidget()->LayoutRootViewIfNecessary();
  }
}

void SidebarContainerViewNew::OnItemAdded(const sidebar::SidebarItem& item,
                                          size_t index,
                                          bool user_gesture) {
  sidebar_control_view_->Update();
  UpdateToolbarButtonVisibility();
}

void SidebarContainerViewNew::OnItemRemoved(size_t index) {
  sidebar_control_view_->Update();
  UpdateToolbarButtonVisibility();
}

bool SidebarContainerViewNew::ShouldUseAnimation() {
  return true;
}

void SidebarContainerViewNew::ShowSidebarControlView() {
  if (!initialized_) {
    return;
  }

  StopBrowserWindowEventMonitoring();

  const bool was_visible = sidebar_control_view_->GetVisible();
  if (!was_visible) {
    sidebar_control_view_->SetVisible(true);
  }

  if (ShouldUseAnimation() && !was_visible) {
    animation_start_width_ = 0;
    animation_end_width_ = sidebar_control_view_->GetPreferredSize().width();
    width_animation_.Show();
  } else if (GetWidget()) {
    GetWidget()->LayoutRootViewIfNecessary();
  }

  UpdateToolbarButtonVisibility();
}

void SidebarContainerViewNew::HideSidebarControlView() {
  if (!initialized_) {
    return;
  }

  sidebar_hide_timer_.Stop();
  StopBrowserWindowEventMonitoring();

  const bool was_visible = sidebar_control_view_->GetVisible();
  if (!was_visible) {
    return;
  }

  if (ShouldUseAnimation()) {
    animation_start_width_ = 0;
    animation_end_width_ = sidebar_control_view_->GetPreferredSize().width();
    width_animation_.Hide();
  } else {
    sidebar_control_view_->SetVisible(false);
    if (GetWidget()) {
      GetWidget()->LayoutRootViewIfNecessary();
    }
  }

  UpdateToolbarButtonVisibility();
}

void SidebarContainerViewNew::HideSidebarControlViewForShowOption() {
  if (show_sidebar_option_ == ShowSidebarOption::kShowAlways) {
    return;
  }

  HideSidebarControlView();
}

bool SidebarContainerViewNew::ShouldForceShowSidebar() const {
  return sidebar_control_view_->IsItemReorderingInProgress() ||
         sidebar_control_view_->IsBubbleWidgetVisible();
}

void SidebarContainerViewNew::UpdateToolbarButtonVisibility() {
  // Coordinate sidebar toolbar button visibility based on
  // whether there are any sibar items with a sidepanel.
  // This is similar to how chromium's side_panel_coordinator View
  // also has some control on the toolbar button.
  auto has_panel_item =
      GetSidebarService(GetBraveBrowser())->GetDefaultPanelItem().has_value();
  auto* browser_view = BrowserView::GetBrowserViewForBrowser(browser_);
  auto* brave_toolbar = static_cast<BraveToolbarView*>(browser_view->toolbar());
  if (brave_toolbar && brave_toolbar->side_panel_button()) {
    brave_toolbar->side_panel_button()->SetVisible(
        has_panel_item && show_side_panel_button_.GetValue());
  }
}

void SidebarContainerViewNew::StartBrowserWindowEventMonitoring() {
  if (browser_window_event_monitor_) {
    return;
  }

  browser_window_event_monitor_ = views::EventMonitor::CreateWindowMonitor(
      browser_window_event_observer_.get(), GetWidget()->GetNativeWindow(),
      {ui::EventType::kMouseMoved});
}

void SidebarContainerViewNew::StopBrowserWindowEventMonitoring() {
  browser_window_event_monitor_.reset();
}

BraveBrowser* SidebarContainerViewNew::GetBraveBrowser() const {
  return static_cast<BraveBrowser*>(browser_.get());
}

BEGIN_METADATA(SidebarContainerViewNew)
END_METADATA
