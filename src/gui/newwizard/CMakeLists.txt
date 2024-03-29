add_library(newwizard STATIC
    enums.cpp

    pages/abstractsetupwizardpage.h
    pages/abstractsetupwizardpage.cpp

    pages/serverurlsetupwizardpage.ui
    pages/serverurlsetupwizardpage.h
    pages/serverurlsetupwizardpage.cpp

    pages/webfingersetupwizardpage.ui
    pages/webfingersetupwizardpage.h
    pages/webfingersetupwizardpage.cpp

    pages/credentialssetupwizardpage.ui

    pages/basiccredentialssetupwizardpage.h
    pages/basiccredentialssetupwizardpage.cpp

    pages/oauthcredentialssetupwizardpage.h
    pages/oauthcredentialssetupwizardpage.cpp

    pages/accountconfiguredwizardpage.ui
    pages/accountconfiguredwizardpage.h
    pages/accountconfiguredwizardpage.cpp

    setupwizardwindow.ui
    setupwizardwindow.h
    setupwizardwindow.cpp

    setupwizardcontroller.h
    setupwizardcontroller.cpp

    setupwizardaccountbuilder.cpp
    setupwizardaccountbuilder.h

    navigation.cpp
    navigation.h

    jobs/resolveurljobfactory.cpp
    jobs/resolveurljobfactory.h

    jobs/checkbasicauthjobfactory.cpp
    jobs/checkbasicauthjobfactory.h

    postfixlineedit.cpp

    states/abstractsetupwizardstate.cpp
    states/serverurlsetupwizardstate.cpp
    states/webfingersetupwizardstate.cpp
    states/basiccredentialssetupwizardstate.cpp
    states/oauthcredentialssetupwizardstate.cpp
    states/accountconfiguredsetupwizardstate.cpp

    setupwizardcontext.cpp
)
target_link_libraries(newwizard PUBLIC Qt5::Widgets libsync)
set_target_properties(newwizard PROPERTIES AUTOUIC ON AUTORCC ON)
target_include_directories(newwizard PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
apply_common_target_settings(newwizard)
