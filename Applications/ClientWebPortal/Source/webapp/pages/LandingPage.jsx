define(function(require) {
  var React = require('react');
  var BurgerButton = require('app/components/BurgerButton');
  var ProfilesPage = require('app/pages/ProfilesPage');
  var ReportsPage = require('app/pages/ReportsPage');

  class Header extends React.Component {
    render() {
      var burgerStyle = {
        float: 'left',
      };
      return (
        <div style = {burgerStyle}>
          <BurgerButton
            width = {30}
            height = {30}
            background_color = {'transparent'}
            stroke_color = {'white'}
            onClick = {this.props.onMenuClick}
          />
        </div>);
    }
  }

  class SideMenu extends React.Component {
    render() {
      var menuStyle = {
        backgroundColor: 'red',
        flexGrow: 0,
        flexShrink: 0,
        flexBasis: 'content',
        display: 'flex',
        flexDirection: 'column'
      };
      return (
        <div style = {menuStyle}>
          <button
              type = "button"
              onClick = {this.props.onProfiles}>
            Profiles
          </button>
          <button
              type = "button"
              onClick = {this.props.onReports}>
            Reports
          </button>
          <button
              type = "button"
              onClick = {this.props.onPortfolio}>
            Portfolio
          </button>
          <button
              type = "button"
              onClick = {this.props.onSignOut}>
            Sign Out
          </button>
        </div>);
    }
  }

  class LandingPage extends React.Component {
    static get PROFILES_PAGE() {
      return 1;
    }

    static get REPORTS_PAGE() {
      return 2;
    }

    static get PORTFOLIO_PAGE() {
      return 3;
    }

    static get SIGNOUT() {
      return 4;
    }

    constructor() {
      super();
      this.state = {
        isMenuDisplayed: false,
        bodyPage: LandingPage.PROFILES_PAGE
      };
      this.toggleMenu = this.toggleMenu.bind(this);
      this.handleProfiles = this.handleProfiles.bind(this);
      this.handleReports = this.handleReports.bind(this);
      this.handleSignOut = this.handleSignOut.bind(this);
    }

    toggleMenu() {
      if(this.state.isMenuDisplayed) {
        this.hideMenu();
      } else {
        this.showMenu();
      }
    }

    showMenu() {
      if(this.state.isMenuDisplayed) {
        return;
      }
      this.setState({isMenuDisplayed: true});
    }

    hideMenu() {
      if(!this.state.isMenuDisplayed) {
        return;
      }
      this.setState({isMenuDisplayed: false});
    }

    handleProfiles() {
      this.setState({bodyPage: LandingPage.PROFILES_PAGE},
        function() {
          this.props.application.advanceHistory(this, '#profiles');
        });
    }

    handleReports() {
      this.setState({bodyPage: LandingPage.REPORTS_PAGE},
        function() {
          this.props.application.advanceHistory(this, '#reports');
        });
    }

    handleSignOut() {
      this.props.application.client.logout();
      this.props.application.handleLoggedOut();
    }

    render() {
      var containerStyle = {
        width: '100%',
        height: '100%',
        margin: 0,
        padding: 0,
        display: 'flex',
        flexDirection: 'column',
        flexWrap: 'nowrap'
      };
      var headerStyle = {
        backgroundColor: 'black',
        flexGrow: 0,
        flexShrink: 0,
        flexBasis: 'content'
      };
      var bodyStyle = {
        flexGrow: 1,
        display: 'flex',
        flexDirection: 'row',
        flexWrap: 'nowrap'
      };
      var contentStyle = {
        backgroundColor: 'green',
        flexGrow: 1,
        flexShink: 0
      };
      return (
        <div style = {containerStyle}>
          <div style = {headerStyle}>
            <Header onMenuClick = {this.toggleMenu} />
          </div>
          <div style = {bodyStyle}>
            {
              function() {
                if(this.state.isMenuDisplayed) {
                  return (
                    <SideMenu
                      onProfiles = {this.handleProfiles}
                      onReports = {this.handleReports}
                      onSignOut = {this.handleSignOut}
                    />);
                } else {
                  return null;
                }
              }.bind(this)()
            }
            <div style = {contentStyle}>
              {
                function() {
                  if(this.state.bodyPage == LandingPage.PROFILES_PAGE) {
                    return (<ProfilesPage
                              application = {this.props.application} />);
                  } else if(this.state.bodyPage == LandingPage.REPORTS_PAGE) {
                    return <ReportsPage />;
                  }
                  return null;
                }.bind(this)()
              }
            </div>
          </div>
        </div>);
    }

    componentDidMount() {
      this.props.application.advanceHistory(this, '#app');
    }
  }
  return LandingPage;
});
