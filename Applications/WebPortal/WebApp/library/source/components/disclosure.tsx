import * as React from 'react';

interface Properties {

  /** Whether the disclosure is expanded. Defaults to false. */
  open?: boolean;

  /** The component that is always visible within the disclosure. */
  header: React.ReactNode;

  /** The component that is visible when the disclosure is open. */
  details: React.ReactNode;

  /** The transition duration in milliseconds. Defaults to 600. */
  transition?: number;

  /** Called when the header is clicked to toggle the open state. */
  onToggle?: (open: boolean) => void;
}

/** A collapsible container with animated open/close transition. */
export class Disclosure extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.contentRef = React.createRef<HTMLDivElement>();
    this.detailsRef = React.createRef<HTMLDivElement>();
    this.maxHeight = (props.open ?? false) ? 'none' : '0px';
  }

  public render(): JSX.Element {
    const duration = this.props.transition ?? 600;
    return (
      <div>
        <div onClick={this.onHeaderClick} style={{cursor: 'pointer'}}>
          {this.props.header}
        </div>
        <div ref={this.contentRef} style={{
            overflow: 'hidden',
            maxHeight: this.maxHeight,
            transition: `max-height ${duration}ms`}}>
          <div ref={this.detailsRef}>
            {this.props.details}
          </div>
        </div>
      </div>);
  }

  public componentDidMount(): void {
    const isOpen = this.props.open ?? false;
    if(isOpen && this.detailsRef.current) {
      this.maxHeight = `${this.detailsRef.current.scrollHeight}px`;
      this.forceUpdate();
      this.startObserver();
    }
  }

  public componentDidUpdate(prevProps: Properties): void {
    const wasOpen = prevProps.open ?? false;
    const isOpen = this.props.open ?? false;
    if(isOpen && !wasOpen) {
      this.startObserver();
      if(this.detailsRef.current) {
        this.maxHeight = `${this.detailsRef.current.scrollHeight}px`;
        this.forceUpdate();
      }
    } else if(!isOpen && wasOpen) {
      this.stopObserver();
      if(this.contentRef.current) {
        this.maxHeight = `${this.contentRef.current.scrollHeight}px`;
        this.forceUpdate();
        requestAnimationFrame(() => {
          this.maxHeight = '0px';
          this.forceUpdate();
        });
      }
    }
  }

  public componentWillUnmount(): void {
    this.stopObserver();
  }

  private startObserver(): void {
    this.stopObserver();
    if(this.detailsRef.current) {
      this.observer = new ResizeObserver(() => {
        if(this.detailsRef.current) {
          this.maxHeight = `${this.detailsRef.current.scrollHeight}px`;
          this.forceUpdate();
        }
      });
      this.observer.observe(this.detailsRef.current);
    }
  }

  private stopObserver(): void {
    this.observer?.disconnect();
    this.observer = null;
  }

  private onHeaderClick = () => {
    const isOpen = this.props.open ?? false;
    this.props.onToggle?.(!isOpen);
  };

  private contentRef: React.RefObject<HTMLDivElement>;
  private detailsRef: React.RefObject<HTMLDivElement>;
  private observer: ResizeObserver;
  private maxHeight: string;
}
