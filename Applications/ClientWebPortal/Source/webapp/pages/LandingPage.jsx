define(function(require) {
  var React = require('react');
  var Radium = require('radium');
  var BurgerButton = require('app/components/BurgerButton');
  var ProfilesPage = require('app/pages/ProfilesPage');
  var ReportsPage = require('app/pages/ReportsPage');

  class Header extends React.Component {
    render() {
      var burgerStyle = {
        flexGrow: 0,
        flexShrink: 0,
        flexBasis: 'content',
        display: 'flex',
        flexDirection: 'row',
        padding: '15px',
        borderBottom: '1px solid #f5f5f5'
      };
      return (
        <div style = {burgerStyle}>
          <BurgerButton
            width = {30}
            height = {30}
            background_color = {"transparent"}
            stroke_color = {"#9ac7ea"}
            onClick = {this.props.onMenuClick}
          />
          <div
            style = {{
              width: "15px",
              height: "15px"
            }}
          />
          <img
            src = "/img/spire_header_logo.png"
            width = "84px"
            height = "30px"
          />
        </div>);
    }
  }

  class MenuButton extends React.Component {
    render() {
      var buttonStyle = {
        backgroundColor: '#2ca1db',
        border: 'none',
        color: 'white',
        height: '35px',
        paddingLeft: '20px',
        ':hover': {
          backgroundColor: '#aadaf0'
        }
      };
      var layoutStyle = {
        flexGrow: 0,
        flexShrink: 0,
        flexBasis: 'content',
        display: 'flex',
        flexDirection: 'row',
        alignItems: 'center',
      };
      return (
        <button
            type = "button"
            style = {buttonStyle}
            onClick = {this.props.onClick}>
          <div style = {layoutStyle}>
            <img
              src = {this.props.icon}
              width = "20px"
              height = "20px"
            />
            <div
              style = {{
                width: "15px"
              }}
            />
            {this.props.itemName}
          </div>
        </button>);
    }
  }

  MenuButton = Radium(MenuButton);

  class SideMenu extends React.Component {
    render() {
      var menuStyle = {
        backgroundColor: '#2ca1db',
        width: '200px',
        flexGrow: 0,
        flexShrink: 0,
        flexBasis: 'content',
        display: 'flex',
        flexDirection: 'column'
      };
      return (
        <div style = {menuStyle}>
          <MenuButton
            icon = "/img/main_menu/profile.svg"
            itemName = "Profiles"
            onClick = {this.props.onProfiles}
          />
          <MenuButton
            icon = "/img/main_menu/reports.svg"
            itemName = "Reports"
            onClick = {this.props.onReports}
          />
          <MenuButton
            icon = "/img/main_menu/portfolio.svg"
            itemName = "Portfolio"
            onClick = {this.props.onPortfolio}
          />
          <MenuButton
            icon = "/img/main_menu/sign_out.svg"
            itemName = "Sign Out"
            onClick = {this.props.onSignOut}
          />
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
        backgroundColor: 'white',
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
        backgroundColor: 'white',
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
