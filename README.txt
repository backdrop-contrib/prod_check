$Id$

README file for the Production check & Production monitor Drupal modules.


Introduction
============

When bringing a site live, you should double check a lot of settings, like the
error logging, site e-mail, disabling the Devel module and so on.
Next to that, you should ensure that all SEO modules are installed and properly
configured (like Google Analytics, Page Title, XML Sitemap etc.). The Production
check module will do all of this checking for you and present the results in a
convenient status page accessible through /admin/reports/prod-check. Through
this status page, you can easily navigate to all the settings pages or the
project pages of the missing modules to rectify all you need to.

It would of course also be nice that these settings remain as you set them up.
In some cases, when multiple developers make updates to a live site or with the
odd client having somehow gotten superadmin access, stuff can get changed,
usually unintended. That's where the Production monitor comes in the picture.
You can open up the Production check's XMLRPC interface through its settings
page and have the Production monitor module connect to it from a 'local'
monitoring site in your development environment. This will allow you to monitor
all your sites from a central server and keep an eye on them. When adding a site
using Production monitor, you can indicate what exactly needs to be monitored
for this site. Updates can be requested manually and are fetched automatically
each cron run.

    "But I like Nagios to monitor my sites!"

If you prefer Nagios monitoring, you can open up Production check's Nagios
integration from its settings page. You can specify what exactly you want to
monitor there. You will obviousely need to install the Nagios module to make
this functionality work.


Dependencies
============

- Nagios   http://drupal.org/project/nagios

There are no true dependencies defined in the .info file, but naturally you need
to install the Nagios module if you would like to integrate Production check
with your Nagios monitoring setup.


Installation
============

Production check
----------------
1. Extract the prod_check module and place it in /sites/all/modules/contrib
2. Remove the 'prod_monitor' folder and all it's contents
3. Open the following files in a text editor:

  prod_check/prod_check.module
  prod_check/includes/prod_check.apc.inc

 Look on line 14 or in prod_check.module and change the part that reads
 '--- change this ---' to (part of) the e-mail address you always use in
 /admin/settings/site-information when developing a website.
 The reason why this was not added to the settings page is that this will allow
 you to upload the module to all your sites that need monitoring without needing
 to adjust this time and again.
 I'm still not sure about this highly unusual approach, so feel free to comment
 on it in the issue queue. Maybe the prod_check settings page is right after all.

 In prod_check.apc.inc, look on line 44 for the part that reads 'password' and
 change this to a secure password to be able to use the advanced features
 supplied by this page.
 This will NOT be added to any settings page in any way in order to comply with
 the licence note at the top of the file.
 Line 66 is the ONLY modification (and frankly in my humble opinion sort of a
 bugfix) to make the integration of this page possible for prod_check.

4. Upload the prod_check folder to the websites you wish to check / monitor,
 enable the module and adjust it's settings using /admin/settings/prod-check.

5. You can check the /admin/reports/status page to verify if the Production
 check setup described above was executed correctly and no errors / warnings are
 reported.

6. You can find the result of the Production check module on
 /admin/reports/prod-check

Production monitor
------------------
1. Grab the prod_monitor folder from the package and upload it to your
 'monitoring site' and activate the module.
2. Make sure that the site you wish to monitor is running the prod_check module
3. Navigate to the prod_check settings page and activate XMLRPC and add an API
 key to 'secure' the connection. The key is limited to 128 characters.
4. Add the site to the Production monitor overview page on
 /admin/reports/prod-monitor
5. Enter the url and the API key and hit 'Get settings'. All available checks
 are now retrieved from the remote site. You can uncheck those that you do not
 wish to monitor.
6. If you wish to fetch the data immediately, check the appropriate box and save
 the settings. Good to go!

Nagios
------
1. Download and install the Nagios module from http://drupal.org/project/nagios
 as per its readme instructions
2. Enable Nagios support in the prod_check module on /admin/settings/prod-check
 by ticking the appropriate box.
3. Untick the checboxes for those items you do not whish to be monitored by
 Nagios.
4. Save the settings and you're good to go!


Updates
=======
When new checks are added to the prod_check module, the prod_monitor module will
automatically fetch them from the remote server when you edit the settings. Upon
displaying the edit form, XMLRPC is ALWAYS used to build op the checkboxes array
so that you always have the latest options available.
Cron is NOT used to do this, since we want to keep the transfer to a minimum.


Hidden link
===========
Production check adds a 'hidden link' to the site where you can check the APC
status of your site. This page can be found on /admin/reports/status/apc.
This is in analogy with the system module that adds these 'hidden pages':
 /admin/reports/status/php
 /admin/reports/status/sql

Truely unmissable when setting up your site on a production server to check if
all is well!


The detailed report page
========================

The page is divided into 4 sections:

 - Settings: checks various Drupal settings
 - Server: checks that are 'outside of Drupal' such as APC and wether or not you
           have removed the release note files from the root.
 - Performance: checks relevant to the performance settings in Drupal such as
                page / block caching.
 - Modules: checks if certain modules are on / off
 - SEO: performs very basic SEO checks such as 'is Google Analytics activated
        and did you provide a GA account number.

The sections might shift over time (maybe some stuff should go under a
'Security' section etc.).

The checks itself should be self explanatory to Drupal developers, so they won't
be described in detail here.


Support
=======

For support requests, bug reports, and feature requests, please us the issue cue
of Menu Clone on http://drupal.org/project/issues/prod_check.

