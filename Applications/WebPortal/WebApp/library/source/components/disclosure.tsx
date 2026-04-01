import * as React from 'react';

interface Properties {

  /** Whether the disclosure is expanded. Defaults to false. */
  open?: boolean;

  /** The component that is always visible within the disclosure. */
  header: React.ReactNode;

  /** The component that is visible when the disclosure is open. */
  details: React.ReactNode;

  /** Called when the header is clicked to toggle the open state. */
  onToggle?: (open: boolean) => void;
}

/** A collapsible container with animated open/close transition. */
export function Disclosure(props: Properties) {
  const isOpen = props.open ?? false;
  const contentRef = React.useRef<HTMLDivElement>(null);
  const detailsRef = React.useRef<HTMLDivElement>(null);
  const [maxHeight, setMaxHeight] = React.useState(isOpen ? 'none' : '0px');
  const isFirstRender = React.useRef(true);
  React.useEffect(() => {
    if(isFirstRender.current) {
      isFirstRender.current = false;
      if(isOpen && detailsRef.current) {
        setMaxHeight(`${detailsRef.current.scrollHeight}px`);
      }
      return;
    }
    if(isOpen) {
      const observer = new ResizeObserver(() => {
        if(detailsRef.current) {
          setMaxHeight(`${detailsRef.current.scrollHeight}px`);
        }
      });
      if(detailsRef.current) {
        observer.observe(detailsRef.current);
        setMaxHeight(`${detailsRef.current.scrollHeight}px`);
      }
      return () => observer.disconnect();
    } else {
      if(contentRef.current) {
        const currentHeight = contentRef.current.scrollHeight;
        setMaxHeight(`${currentHeight}px`);
        requestAnimationFrame(() => {
          setMaxHeight('0px');
        });
      }
    }
  }, [isOpen]);
  const onHeaderClick = () => {
    props.onToggle?.(!isOpen);
  };
  return (
    <div>
      <div onClick={onHeaderClick} style={{cursor: 'pointer'}}>
        {props.header}
      </div>
      <div ref={contentRef} style={{
          overflow: 'hidden',
          maxHeight,
          transition: 'max-height 600ms'}}>
        <div ref={detailsRef}>
          {props.details}
        </div>
      </div>
    </div>);
}
