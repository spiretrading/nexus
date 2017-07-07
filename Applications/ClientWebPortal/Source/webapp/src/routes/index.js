import App from 'containers/App';
import deviceDetector from 'utils/device-detector';
import desktopRoutes from './desktop';
import mobileRoutes from './mobile';

var isMobile = deviceDetector.isMobile();

let childRoutes;

if (!isMobile) {
  childRoutes = desktopRoutes.getRoutes();
} else {
  childRoutes = mobileRoutes.getRoutes();
}

export default {
  component: App,
  childRoutes: childRoutes
};
