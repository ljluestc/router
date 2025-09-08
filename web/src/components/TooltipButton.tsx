import React from 'react';
import {
  Button,
  ButtonProps,
  Tooltip,
  TooltipProps,
} from '@mui/material';

interface TooltipButtonProps extends ButtonProps {
  tooltip?: string;
  tooltipProps?: Omit<TooltipProps, 'children' | 'title'>;
  disabled?: boolean;
  disabledTooltip?: string;
}

const TooltipButton: React.FC<TooltipButtonProps> = ({
  tooltip,
  tooltipProps,
  disabled = false,
  disabledTooltip,
  children,
  ...buttonProps
}) => {
  const getTooltipContent = () => {
    if (disabled && disabledTooltip) {
      return disabledTooltip;
    }
    return tooltip;
  };

  const getTooltipProps = () => {
    const baseProps = {
      ...tooltipProps,
    };

    if (disabled && disabledTooltip) {
      return {
        ...baseProps,
        arrow: true,
        placement: 'top' as const,
      };
    }

    return baseProps;
  };

  const tooltipContent = getTooltipContent();

  if (!tooltipContent) {
    return (
      <Button
        {...buttonProps}
        disabled={disabled}
      >
        {children}
      </Button>
    );
  }

  return (
    <Tooltip
      title={tooltipContent}
      {...getTooltipProps()}
    >
      <span>
        <Button
          {...buttonProps}
          disabled={disabled}
        >
          {children}
        </Button>
      </span>
    </Tooltip>
  );
};

export default TooltipButton;
