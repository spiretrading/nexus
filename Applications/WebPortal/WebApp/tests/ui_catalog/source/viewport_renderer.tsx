import * as React from 'react';
import {Viewport} from './viewport';

type Omit<T, K> = Pick<T, Exclude<keyof T, K>>;
type Subtract<T, K> = Omit<T, keyof K>;

/** The properties that the BaseComponent needs to have. */
interface Properties {
  viewport: Viewport;
}

interface State {
  viewport: Viewport;
}

/**
 * Returns a component that monitors the viewport size.
 * @param BaseComponent - A component that has a Viewport that needs
 *        to be updated on a viewport size change.
 */
export function viewportRenderer<T extends Properties>(
    BaseComponent: React.ComponentType<T>) {
  const MEDIA_QUERY_LIST = {
    [Viewport.SMALL]: matchMedia('(max-width: 767px)'),
    [Viewport.MEDIUM]: matchMedia(
      '(min-width: 768px) and (max-width: 1279px)'),
    [Viewport.LARGE]: matchMedia('(min-width: 1280px)')
  };
  return class extends React.Component<Subtract<T, Properties>,
      State> {
    constructor(props: T) {
      super(props);
      this.state = {
        viewport: this.getViewport()
      };
    }

    public componentDidMount(): void {
      const onMatchesSmall = this.getMediaQueryListener(Viewport.SMALL);
      const onMatchesMedium = this.getMediaQueryListener(Viewport.MEDIUM);
      const onMatchesLarge = this.getMediaQueryListener(Viewport.LARGE);
      this.mediaQueries = [
        [MEDIA_QUERY_LIST[Viewport.SMALL], onMatchesSmall],
        [MEDIA_QUERY_LIST[Viewport.MEDIUM], onMatchesMedium],
        [MEDIA_QUERY_LIST[Viewport.LARGE], onMatchesLarge]
      ];
      for(const [mediaQueryList, listener] of this.mediaQueries) {
        mediaQueryList.addListener(listener);
      }
    }

    public componentWillUnmount(): void {
      for(const [mediaQueryList, listener] of this.mediaQueries) {
        mediaQueryList.removeListener(listener);
      }
    }

    public render(): JSX.Element {
      const C = BaseComponent as any;
      return <C {...this.props} viewport={this.state.viewport}/>;
    }

    public getMediaQueryListener = (viewport: Viewport) => {
      return (mql: MediaQueryListEvent) => {
        if(mql.matches) {
          this.setState({viewport});
        }
      };
    }

    public getViewport(): Viewport {
      return [Viewport.SMALL, Viewport.MEDIUM, Viewport.LARGE].find(
        (viewport) => MEDIA_QUERY_LIST[viewport].matches);
    }

    public mediaQueries: [MediaQueryList, (ev: MediaQueryListEvent) => void][];
  };
}
