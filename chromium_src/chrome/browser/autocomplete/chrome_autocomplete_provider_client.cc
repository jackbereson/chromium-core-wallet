// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "src/chrome/browser/autocomplete/chrome_autocomplete_provider_client.cc"

#include "brave/components/ai_chat/content/browser/ai_chat_tab_helper.h"
#include "brave/components/commander/common/buildflags/buildflags.h"
#include "build/build_config.h"
#include "chrome/browser/profiles/profile.h"

#if !BUILDFLAG(IS_ANDROID)
#include "brave/browser/brave_browser_process.h"
#include "brave/browser/misc_metrics/process_misc_metrics.h"
#include "brave/browser/ui/brave_browser.h"
#include "brave/browser/ui/sidebar/sidebar_controller.h"
#endif  // BUILDFLAG(!IS_ANDROID)

#if BUILDFLAG(ENABLE_COMMANDER)
#include "brave/browser/ui/commander/commander_service_factory.h"
#include "brave/components/commander/browser/commander_frontend_delegate.h"
#endif  // BUILDFLAG(ENABLE_COMMANDER)

#if BUILDFLAG(ENABLE_AI_CHAT)
#include "brave/components/ai_chat/core/browser/ai_chat_metrics.h"
#include "brave/components/ai_chat/core/common/features.h"
#include "brave/components/ai_chat/core/common/pref_names.h"
#endif  // BUILDFLAG(ENABLE_AI_CHAT)

#if BUILDFLAG(ENABLE_COMMANDER)
commander::CommanderFrontendDelegate*
ChromeAutocompleteProviderClient::GetCommanderDelegate() {
  return commander::CommanderServiceFactory::GetForBrowserContext(profile_);
}
#endif  // BUILDFLAG(ENABLE_COMMANDER)

#if BUILDFLAG(ENABLE_AI_CHAT)
void ChromeAutocompleteProviderClient::OpenLeo(const std::u16string& query) {
#if !BUILDFLAG(IS_ANDROID)
  DCHECK(base::FeatureList::IsEnabled(ai_chat::features::kAIChat));

  // Note that we're getting the last active browser. This is what upstream
  // does when they open the history journey from the omnibox. This seem to be
  // good enough because
  // * The time between the user typing and the journey opening is very small,
  // so active browser is unlikely to be changed
  // * Even if the active browser is changed, it'd be better to open the Leo in
  // the new active browser.
  CHECK(profile_->IsRegularProfile());
  Browser* browser =
      chrome::FindTabbedBrowser(profile_,
                                /*match_original_profiles=*/true);
  if (!browser) {
    return;
  }

  auto* chat_tab_helper = ai_chat::AIChatTabHelper::FromWebContents(
      browser->tab_strip_model()->GetActiveWebContents());
  DCHECK(chat_tab_helper);

  // Before trying to activate the panel, unlink page content if needed.
  // This needs to be called before activating the panel to check against the
  // current state.
  chat_tab_helper->MaybeUnlinkPageContent();

  // Activate the panel.
  auto* sidebar_controller =
      static_cast<BraveBrowser*>(browser)->sidebar_controller();
  sidebar_controller->ActivatePanelItem(
      sidebar::SidebarItem::BuiltInItemType::kChatUI);

  // Send the query to the AIChat's backend.
  ai_chat::mojom::ConversationTurnPtr turn =
      ai_chat::mojom::ConversationTurn::New(
          ai_chat::mojom::CharacterType::HUMAN,
          ai_chat::mojom::ActionType::QUERY,
          ai_chat::mojom::ConversationTurnVisibility::VISIBLE,
          base::UTF16ToUTF8(query) /* text */, std::nullopt /* selected_text */,
          std::nullopt /* events */, base::Time::Now(),
          std::nullopt /* edits */);

  chat_tab_helper->SubmitHumanConversationEntry(std::move(turn));

  ai_chat::AIChatMetrics* metrics =
      g_brave_browser_process->process_misc_metrics()->ai_chat_metrics();
  CHECK(metrics);
  metrics->RecordOmniboxOpen();
#endif
}

bool ChromeAutocompleteProviderClient::IsLeoProviderEnabled() {
#if BUILDFLAG(IS_ANDROID)
  return false;
#else
  return profile_->IsRegularProfile() &&
         GetPrefs()->GetBoolean(
             ai_chat::prefs::kBraveChatAutocompleteProviderEnabled);
#endif
}
#endif  // BUILDFLAG(ENABLE_AI_CHAT)
