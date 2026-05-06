CSS Injector
============

Allows administrators to inject CSS into the page output based on configurable
rules. It's useful for adding simple CSS tweaks without modifying a site's
official theme -- for example, a 'nighttime' color scheme could be added during
certain hours. The CSS is added using Backdrop's standard backdrop_add_css()
function and respects page caching, etc.

This module is definitely not a replacement for full-fledged theming, but it
provides site administrators with a quick and easy way of tweaking things
without diving into full-fledged theme hacking.

The rules provided by CSS Injector typically are loaded last, even after the
theme CSS, although another module could override these.

Installation
------------

- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/user-guide/modules

- Visit the configuration page under Administration > Development > CSS Injector
  (admin/config/development/css-injector) to add CSS rules and to configure 
  the settings.

Issues
------

Bugs and Feature requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/css_injector/issues

Current Maintainers
-------------------

- [Vincent Stemen](https://github.com/vstemen)
- [klonos](https://github.com/klonos)
- [Martin Price](https://github.com/yorkshire-pudding)

Credits
-------

- Ported to Backdrop CMS by Vincent Stemen (https://github.com/vstemen)
- Drupal module maintainer: Kirill Borzov (https://www.drupal.org/u/oresh)
- Drupal module maintainer: Kevin Reynen (https://www.drupal.org/u/kreynen)
- Drupal module maintainer: Randy Fay (https://www.drupal.org/u/rfay)
- Drupal module maintainer: Jeff Eaton (https://www.drupal.org/u/eaton)
- Drupal module maintainer: Addison Berry (https://www.drupal.org/u/add1sun)

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.
