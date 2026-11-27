import datetime
import os
from urllib.parse import urlsplit

import yaml

# Configuration for the Sphinx documentation builder.
# All configuration specific to your project should be done in this file.
#
# If you're new to Sphinx and don't want any advanced or custom features,
# just go through the items marked 'TODO'.
#
# A complete list of built-in Sphinx configuration values:
# https://www.sphinx-doc.org/en/master/usage/configuration.html
#
# Ulwazi provides Canonical configuration defaults directly; it does not
# require the canonical-sphinx-config extension.


#######################
# Project information #
#######################

# Project name
#
# TODO: Update with the official name of your project or product

project = "Ubuntu Frame"
author = "Canonical Ltd."


# Sidebar documentation title; best kept reasonably short
#
# TODO: To include a version number, add it here (hardcoded or automated).
version = os.getenv("READTHEDOCS_VERSION", "local")


# TODO: To disable the title, set to an empty string.

html_title = project + " documentation"


# Copyright string; shown at the bottom of the page
#
# Now, the starter pack uses CC-BY-SA as the license
# and the current year as the copyright year.
#
# TODO: If your docs need another license, specify it instead of 'CC-BY-SA'.
#
# TODO: If your documentation is a part of the code repository of your project,
#       it inherits the code license instead; specify it instead of 'CC-BY-SA'.
#
# NOTE: For static works, it is common to provide the first publication year.
#       Another option is to provide both the first year of publication
#       and the current year, especially for docs that frequently change,
#       e.g. 2022–2023 (note the en-dash).
#
#       A way to check a repo's creation date is to get a classic GitHub token
#       with 'repo' permissions; see https://github.com/settings/tokens
#       Next, use 'curl' and 'jq' to extract the date from the API's output:
#
#       curl -H 'Authorization: token <TOKEN>' \
#         -H 'Accept: application/vnd.github.v3.raw' \
#         https://api.github.com/repos/canonical/<REPO> | jq '.created_at'

copyright = f"{datetime.date.today().year}"


# Documentation website URL
#
# TODO: Update with the official URL of your docs or leave empty if unsure.
#
# NOTE: The Open Graph Protocol (OGP) enhances page display in a social graph
#       and is used by social media platforms; see https://ogp.me/

ogp_site_url = f"https://ubuntu.com/frame/docs/{version}/"


# Preview name of the documentation website
#
# TODO: To use a different name for the project in previews, update as needed.

ogp_site_name = project


# Preview image URL
#
# TODO: To customise the preview image, update as needed.

ogp_image = "https://assets.ubuntu.com/v1/cc828679-docs_illustration.svg"


# Product favicon; shown in bookmarks, browser tabs, etc.

# TODO: To customise the favicon, uncomment and update as needed.

# html_favicon = '.sphinx/_static/favicon.png'


# Dictionary of values to pass into the Sphinx context for all pages:
# https://www.sphinx-doc.org/en/master/usage/configuration.html#confval-html_context

# TODO: Adjust to point to the repository where your documentation source files
# are stored.

github_repo = "https://github.com/canonical/ubuntu-frame"

# TODO: Change if your default branch is not 'main'.
repo_default_branch = "main"

# Determine the source branch for GitHub links (view/edit source, license, etc.)
# On Read the Docs, the Git identifier is a branch only for branch builds;
# for PR previews it is a number, and tags cannot be edited on GitHub.
if os.environ.get("READTHEDOCS_VERSION_TYPE") == "branch":
    source_branch = os.environ.get("READTHEDOCS_GIT_IDENTIFIER") or repo_default_branch
else:
    source_branch = repo_default_branch

# TODO: Select the default syntax for docs source files.
# This is for a fallback view/edit source code buttons.

default_source_extension = ".md"

# TODO: Change to your product website URL,
#       dropping the 'https://' prefix, e.g. 'ubuntu.com/lxd'.
#       If there is no such website - set to '/' or remove the {{ product_page }}
#       link from the page header template.

product_page = "ubuntu.com/frame"

html_context = {
    # Product page URL; can be different from product docs URL
    #
    # TODO: If there's no such website,
    #       remove the {{ product_page }} link from the page header template
    #       (usually .sphinx/_templates/header.html; also, see README.rst).
    "product_page": product_page,
    # Product tag image; the orange part of your logo, shown in the page header
    #
    # TODO: To add a tag image, uncomment and update as needed.
    # 'product_tag': '_static/tag.png',
    #
    # Inherit project name
    "project": project,
    # Inherit the author value
    "author": author,
    # Licensing information
    #
    # TODO: Change your product's license name and a link to its file.
    # For the name, we recommend using the standard shorthand identifier from
    # https://spdx.org/licenses
    # For the URL, link directly to the product's license statement, typically found on
    # the product's home page or in its GitHub project.
    "license": {
        "name": "GPL-3.0-only",
        "url": github_repo + "/blob/" + source_branch + "/LICENSE",
    },
    # Google Tag Manager ID (defaults to Canonical's GTM ID).
    # "tag_id": "<google_tag_id>",

    # Your Discourse instance URL
    #
    # TODO: Change to your Discourse instance URL or leave empty.
    #
    # NOTE: If set, adding ':discourse: 123' to an .rst file
    #       will add a link to Discourse topic 123 at the bottom of the page.
    "discourse": "https://discourse.ubuntu.com/c/project/mir/15",
    # Your Mattermost channel URL
    #
    # TODO: Change to your Mattermost channel URL or leave empty.
    "mattermost": "https://chat.canonical.com/canonical/channels/mir-internal",
    # Your Matrix channel URL
    #
    # TODO: Change to your Matrix channel URL or leave empty.
    "matrix": "https://matrix.to/#/#mir-server:matrix.org",
    # Your documentation GitHub repository URL
    #
    # TODO: Change to your documentation GitHub repository URL or leave empty.
    #
    # NOTE: If set, links for viewing the documentation source files
    #       and creating GitHub issues are added at the bottom of each page.
    "github_url": github_repo,
    # Docs branch in the repo; used in links for viewing the source files
    #
    "repo_branch": source_branch,
    # Docs location in the repo; used in links for viewing the source files
    #
    # TODO: To customise the directory, uncomment and update as needed.
    "repo_folder": "/docs/",
    # TODO: To enable or disable the Previous / Next buttons at the bottom of pages
    # Valid options: none, prev, next, both
    # "sequential_nav": "both",
    # TODO: To enable listing contributors on individual pages, set to True
    "display_contributors": False,
    # Required for feedback button
    "feedback": True,
    "github_issues": "enabled",
    "default_source_extension": default_source_extension,
    "default_edit_url": github_repo
    + "/edit/"
    + source_branch
    + "/docs/index"
    + default_source_extension,
    "default_view_url": github_repo
    + "/blob/"
    + source_branch
    + "/docs/index"
    + default_source_extension,
    # Horizontal Nav Menu
    "company": "Canonical",
    # "link1_URL": "https://canonical-starter-pack.readthedocs-hosted.com/",
    # "link1_name": "First optional link",
    # "link2_URL": "https://canonical-starter-pack.readthedocs-hosted.com/",
    # "link2_name": "Second optional link",
    # Canonical Product menu
    # Uncomment if you need a product menu added on the top of every page
    # "add_product_menu": True,
    # Main Horizontal menu
    # "is_docs": False, # Purpose unknown
    "logo_link_URL": "https://documentation.ubuntu.com",
    "logo_img_URL": "https://assets.ubuntu.com/v1/82818827-CoF_white.svg",
    "logo_title": "Canonical",
    # TODO: Customize the footer.
    "footer": {
        # Whether to add the product name as the first entry.
        "product": True,
        # Whether to add the license as the second entry.
        "license": True,
        # List your footer entries. Accepts HTML tags.
        "entries": [
            '<a class="js-revoke-cookie-manager" href="#tracker-settings">Manage your tracker settings</a>',
        ],
    },
}

# TODO: To enable the edit button on pages, uncomment and change the link to a
# public repository on GitHub or Launchpad. Any of the following link domains
# are accepted:
# - https://github.com/example-org/example"
# - https://launchpad.net/example
# - https://git.launchpad.net/example
#
# html_theme_options = {
# 'source_edit_link': 'https://github.com/canonical/sphinx-docs-starter-pack',
# }

html_extra_path = ["robots.txt", "sitemapindex.xml"]

# TODO: Set the path segment for your own project's docs site. For example,
# "ulwazi" corresponds to https://documentation.ubuntu.com/ulwazi/.
# Ulwazi uses this to compute the bundled 404 page's URL prefix.
slug = 'frame/docs'

# Allow opt-in build of the OpenAPI "Hello" example so docs stay clean by default.
if os.getenv("OPENAPI", ""):
    tags.add("openapi")
    html_extra_path.append("how-to/assets/openapi.yaml")

# Limit the number of levels for Table of contents
localtoc_max_depth = 3

#######################
# Sitemap configuration: https://sphinx-sitemap.readthedocs.io/
#######################

# Base URL of RTD hosted project

html_baseurl = f"https://ubuntu.com/frame/docs/"

# URL scheme: sphinx-sitemap appends it to html_baseurl, so it must contain
# only the path segments that Read the Docs serves after the project root.
# READTHEDOCS_VERSION and READTHEDOCS_LANGUAGE are set on every RTD build, even
# for single-version projects whose URLs have no version or language segment,
# so check whether they end READTHEDOCS_CANONICAL_URL. This works for
# single-version, versioned, and translated projects.
# Manual (non-RTD) builds use a placeholder segment.

if "READTHEDOCS_VERSION" in os.environ:
    version = os.environ["READTHEDOCS_VERSION"]
    rtd_url_path = urlsplit(os.environ.get("READTHEDOCS_CANONICAL_URL", "")).path
    rtd_url_segments = [segment for segment in rtd_url_path.split("/") if segment]
    sitemap_url_scheme = "{link}"
    if rtd_url_segments[-1:] == [version]:
        sitemap_url_scheme = "{version}{link}"
        if rtd_url_segments[-2:-1] == [os.environ.get("READTHEDOCS_LANGUAGE")]:
            sitemap_url_scheme = rtd_url_segments[-2] + "/{version}{link}"
else:
    sitemap_url_scheme = "{link}"

# Include `lastmod` dates in the sitemap:

sitemap_show_lastmod = True

# Exclude the generated 404 page from the sitemap:

sitemap_excludes = [
    "404/",
    "genindex/",
    "search/",
]

#######################
# Template and asset locations
#######################

html_theme = "ulwazi"
html_static_path = ["_static"]
templates_path = ["_templates"]


#############
# Redirects #
#############

# To set up redirects: https://documatt.gitlab.io/sphinx-reredirects/usage.html
# For example: 'explanation/old-name.html': '../how-to/prettify.html',

# To set up redirects in the Read the Docs project dashboard:
# https://docs.readthedocs.io/en/stable/guides/redirects.html

# NOTE: If undefined, set to None, or empty,
#       the sphinx_reredirects extension will be disabled.

redirects = {}

rediraffe_redirects = "redirects.txt"

# Strips '/index.html' from destination URLs when building with 'dirhtml'
rediraffe_dir_only = True


###########################
# Link checker exceptions #
###########################

# A regex list of URLs that are ignored by 'make linkcheck'
#
# TODO: Remove or adjust the ACME entry after you update the contributing guide

linkcheck_ignore = [
    "http://127.0.0.1:9400",
    "https://ubuntu.com/.*#get-in-touch",
    "https://matrix.to/#/#mir-server:matrix.org",
    "https://www.realvnc.com/.*",
    "https://www.freedesktop.org/", # has bot protections in place
    "https://www.winehq.org/", # has bot protections in place
    "https://gitlab.gnome.org/",
    "https://registry.khronos.org/", # 403, likely bot protections
    ]


# A regex list of URLs where anchors are ignored by 'make linkcheck'

linkcheck_anchors_ignore_for_url = [r"https://github\.com/.*"]

# give linkcheck multiple tries on failure
# linkcheck_timeout = 30
linkcheck_retries = 3

########################
# Configuration extras #
########################

# Custom MyST syntax extensions; see
# https://myst-parser.readthedocs.io/en/latest/syntax/optional.html
#
# NOTE: By default, the following MyST extensions are enabled:
#       substitution, deflist, linkify

myst_enable_extensions = {
    "attrs_block",
    "attrs_inline",
    "colon_fence",
    "deflist",
    "substitution",
    "tasklist",
}

# Enable internal cross-references
myst_heading_anchors = 4

# Custom Sphinx extensions; see
# https://www.sphinx-doc.org/en/master/usage/extensions/index.html

# NOTE: The Ulwazi theme ("ulwazi") provides the Canonical configuration
#       defaults that used to come from the canonical_sphinx_config extension.
#
# NOTE: "ulwazi" must stay listed before "sphinx_modern_pdf_style": Sphinx fires
#       'config-inited' in registration order, and the theme injects the
#       Canonical PDF branding defaults that sphinx_modern_pdf_style reads.

extensions = [
    "sphinx_terminal",
    "sphinxcontrib.cairosvgconverter",
    "sphinx_last_updated_by_git",
    "sphinx.ext.intersphinx",
    "sphinx_sitemap",
    "ulwazi",
    "myst_parser",
    "sphinxcontrib.jquery",
    "sphinx_design",
    "sphinx.ext.autosectionlabel",
    "notfound.extension",
    "sphinx_rerediraffe",
    "sphinx_reredirects",
    "sphinx_tabs.tabs",
    "sphinxext.opengraph",
    "sphinx_config_options",
    "sphinx_contributor_listing",
    "sphinx_filtered_toctree",
    "sphinx_related_links",
    "sphinx_roles",
    "sphinx_ubuntu_images",
    "sphinx_youtube_links",

]

# Excludes files or directories from processing

exclude_patterns = [
    "doc-cheat-sheet*",
    "_build",
    "Thumbs.db",
    ".DS_Store",
    ".venv"
]

# Adds custom CSS files, located under 'html_static_path'

html_css_files = [
    "https://assets.ubuntu.com/v1/d86746ef-cookie_banner.css",
]


# Adds custom JavaScript files, located under 'html_static_path'

html_js_files = [
    "js/overwrite_links.js",
    "https://assets.ubuntu.com/v1/287a5e8f-bundle.js",
]


# Syntax highlighting settings

highlight_language = "none"  # default
pygments_style = "autumn"  # see https://pygments.org/styles for more
pygments_dark_style = "github-dark"  # see https://pygments.org/styles for more

# Specifies a reST snippet to be appended to each .rst file

rst_epilog = """
.. include:: /reuse/links.txt
.. include:: /reuse/substitutions.txt
"""

# Feedback button at the top; enabled by default
#
# TODO: To disable the button, uncomment this.

# disable_feedback_button = True


# Your manpage URL
#
# TODO: To enable manpage links, uncomment and replace {codename} with required
#       release, preferably an LTS release (e.g. noble). Do *not* substitute
#       {section} or {page}; these will be replaced by sphinx at build time
#
# NOTE: If set, adding ':manpage:' to an .rst file
#       adds a link to the corresponding man section at the bottom of the page.

# manpages_url = 'https://manpages.ubuntu.com/manpages/{codename}/en/' + \
#     'man{section}/{page}.{section}.html'


# Specifies a reST snippet to be prepended to each .rst file
# This defines a :center: role that centers table cell content.
# This defines a :h2: role that styles content for use with PDF generation.

rst_prolog = """
.. role:: center
   :class: align-center
.. role:: h2
    :class: hclass2
.. role:: woke-ignore
    :class: woke-ignore
.. role:: vale-ignore
    :class: vale-ignore
"""

# Workaround for https://github.com/canonical/canonical-sphinx/issues/34

if "discourse_prefix" not in html_context and "discourse" in html_context:
    html_context["discourse_prefix"] = html_context["discourse"] + "/t/"

# Workaround for substitutions.yaml

if os.path.exists("./reuse/substitutions.yaml"):
    with open("./reuse/substitutions.yaml", "r") as fd:
        myst_substitutions = yaml.safe_load(fd.read())

# Add configuration for intersphinx mapping

intersphinx_mapping = {
    "starter-pack": (
        "https://canonical-example-product-documentation.readthedocs-hosted.com/en/latest",
        None,
    ),
    "mir": ("https://canonical.com/mir/docs/page", None),
    "mir-latest": ("https://canonical.com/mir/docs/latest", None),
    "core": ("https://documentation.ubuntu.com/core", None),
    "frame-22": ("https://ubuntu.com/frame/docs/22", None),
    "server": ("https://ubuntu.com/server/docs", None),
    "snapcraft": ("https://documentation.ubuntu.com/snapcraft/stable", None),
    "snap": ("https://snapcraft.io/docs/", None),
    "subiquity": (
        "https://canonical-subiquity.readthedocs-hosted.com/en/latest",
        None,
    ),
}

# Limit autosection label depth and prefix document names for disambiguation
autosectionlabel_prefix_document = True
autosectionlabel_maxdepth = 2

# PDF

set_modern_pdf_config = True
