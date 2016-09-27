import App from 'containers/App';
import deviceDetector from 'utils/device-detector';
import desktopRoutes from './desktop';
import mobileRoutes from './mobile';

var isMobile = deviceDetector.isMobile();

let childRoutes;

if (!isMobile) {
  childRoutes = desktopRoutes.getRoutes.apply(desktopRoutes);
} else {
  childRoutes = mobileRoutes.getRoutes.apply(mobileRoutes);
}

export default {
  component: App,
  childRoutes: childRoutes
};
