/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/serving/permission_rules/promoted_content_ads/promoted_content_ads_per_hour_permission_rule.h"

#include "base/time/time.h"
#include "brave/components/brave_ads/core/internal/ad_units/inline_content_ad/inline_content_ad_feature.h"
#include "brave/components/brave_ads/core/internal/ad_units/promoted_content_ad/promoted_content_ad_feature.h"
#include "brave/components/brave_ads/core/internal/common/logging_util.h"
#include "brave/components/brave_ads/core/internal/common/time/time_constraint_util.h"
#include "brave/components/brave_ads/core/public/ad_units/ad_type.h"

namespace brave_ads {

bool HasPromotedContentAdsPerHourPermission() {
  if (!DoesHistoryRespectRollingTimeConstraint(
          AdType::kPromotedContentAd, /*time_constraint=*/base::Hours(1),
          /*cap=*/kMaximumPromotedContentAdsPerHour.Get())) {
    BLOG(2, "You have exceeded the allowed promoted content ads per hour");
    return false;
  }

  return true;
}

}  // namespace brave_ads
