/*
 * Copyright 2014 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Author: stevensr@google.com (Ryan Stevens)

#ifndef NET_INSTAWEB_REWRITER_PUBLIC_MOBILIZE_REWRITE_FILTER_H_
#define NET_INSTAWEB_REWRITER_PUBLIC_MOBILIZE_REWRITE_FILTER_H_

#include "net/instaweb/rewriter/public/common_filter.h"
#include "net/instaweb/rewriter/public/rewrite_driver.h"
#include "net/instaweb/rewriter/public/rewrite_options.h"
#include "net/instaweb/rewriter/public/static_asset_manager.h"
#include "pagespeed/kernel/base/basictypes.h"
#include "pagespeed/kernel/base/statistics.h"
#include "pagespeed/kernel/base/string.h"
#include "pagespeed/kernel/base/string_util.h"
#include "pagespeed/kernel/html/html_element.h"
#include "pagespeed/kernel/html/html_filter.h"
#include "pagespeed/kernel/http/user_agent_matcher.h"

namespace net_instaweb {

// Rewrite HTML to be mobile-friendly based on "data-mobile-role" attributes in
// the HTML tags. To reorganize the DOM, the filter puts containers at the end
// of the body into which we move tagged elements. The containers are later
// removed after the filter is done processing the document body. The filter
// applies the following transformations:
//  - Add mobile <style> and <meta name="viewport"...> tags to the head.
//  - Remove all table tags (but keep the content). Almost all tables in desktop
//    HTML are for formatting, not displaying data, and they tend not to resize
//    well for mobile. The easiest thing to do is to simply strip out the
//    formatting and hope the content reflows properly.
//  - Reorder body of the HTML DOM elements based on mobile role. Any elements
//    which don't have an important parent will get removed, except for a
//    special set of "keeper" tags (like <script> or <style>). The keeper tags
//    are retained because they are often necessary for the website to work
//    properly, and because they have no visible appearance on the page.
//  - Remove all elements from inside data-mobile-role="navigational" elements
//    except in a special set of nav tags (notably <a>). Nav sections often do
//    not resize well due to fixed width formatting and drop-down menus, so it
//    is often necessary to pull out what you want, instead of shuffling around
//    what is there.
//
// Remaining todos:
//  - TODO (stevensr): This script does not handle flush windows in the body.
//  - TODO (stevensr): It would be nice to tweak the table-xform behavior via
//    options. Also, there has been mention that removing tables across flush
//    windows could be problematic. This should be addressed at some point.
//  - TODO (stevensr): Enable this filter only for mobile UAs, and have a query
//    param option to turn it on for all UAs for debugging.
//  - TODO (stevensr): Write pcache entry if rewriting page fails. We should
//    then probably inject some JS to auto-refresh the page so the user does not
//    see the badly rewritten result.
//  - TODO (stevensr): Add a separate wildcard option to allow/disallow URLs
//    from using this filter. Of course sites can use our existing Allow and
//    Disallow directives but that turns off all optimizations, and this one is
//    one that might be extra finicky (e.g. don't touch my admin pages).
//  - TODO (stevensr): Turn on css_move_to_head_filter.cc to reorder elements
//    we inject into the head.
class MobilizeRewriteFilter : public CommonFilter {
 public:
  static const char kPagesMobilized[];

  explicit MobilizeRewriteFilter(RewriteDriver* rewrite_driver);
  virtual ~MobilizeRewriteFilter();

  static void InitStats(Statistics* statistics);

  // True if options or request UA suggest we will actually do mobilization.
  static bool IsApplicableFor(RewriteDriver* driver);
  static bool IsApplicableFor(const RewriteOptions* options,
                              const char* user_agent,
                              const UserAgentMatcher* matcher);
  virtual void DetermineEnabled(GoogleString* disabled_reason);
  virtual void StartDocumentImpl();
  virtual void EndDocument();
  virtual void RenderDone();
  virtual void StartElementImpl(HtmlElement* element);
  virtual void EndElementImpl(HtmlElement* element);
  virtual const char* Name() const { return "MobilizeRewrite"; }
  ScriptUsage GetScriptUsage() const override { return kWillInjectScripts; }

 private:
  void AppendStylesheet(StringPiece css_file_name,
                        StaticAssetEnum::StaticAsset asset,
                        HtmlElement* element);
  void AddStyle(HtmlElement* element);

  GoogleString GetMobJsInitScript();

  int body_element_depth_;
  bool added_viewport_;
  bool added_style_;
  bool added_spacer_;
  bool saw_end_document_;

  // Statistics
  // Number of web pages we have mobilized.
  Variable* num_pages_mobilized_;

  // Used for overriding default behavior in testing.
  friend class MobilizeRewriteFilterTest;

  DISALLOW_COPY_AND_ASSIGN(MobilizeRewriteFilter);
};

}  // namespace net_instaweb

#endif  // NET_INSTAWEB_REWRITER_PUBLIC_MOBILIZE_REWRITE_FILTER_H_
