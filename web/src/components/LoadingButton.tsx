import React from 'react';
import {
  Button,
  ButtonProps,
  CircularProgress,
  Box,
} from '@mui/material';

interface LoadingButtonProps extends Omit<ButtonProps, 'disabled'> {
  loading?: boolean;
  loadingText?: string;
  loadingSize?: number;
  loadingPosition?: 'start' | 'end' | 'center';
}

const LoadingButton: React.FC<LoadingButtonProps> = ({
  loading = false,
  loadingText,
  loadingSize = 20,
  loadingPosition = 'start',
  children,
  disabled,
  startIcon,
  endIcon,
  ...props
}) => {
  const getLoadingIcon = () => (
    <CircularProgress
      size={loadingSize}
      color="inherit"
      sx={{
        position: loadingPosition === 'center' ? 'absolute' : 'static',
        top: loadingPosition === 'center' ? '50%' : 'auto',
        left: loadingPosition === 'center' ? '50%' : 'auto',
        transform: loadingPosition === 'center' ? 'translate(-50%, -50%)' : 'none',
      }}
    />
  );

  const getContent = () => {
    if (loading && loadingPosition === 'center') {
      return (
        <Box sx={{ opacity: 0 }}>
          {children}
        </Box>
      );
    }

    if (loading && loadingPosition === 'start') {
      return (
        <Box display="flex" alignItems="center" gap={1}>
          {getLoadingIcon()}
          {loadingText || children}
        </Box>
      );
    }

    if (loading && loadingPosition === 'end') {
      return (
        <Box display="flex" alignItems="center" gap={1}>
          {loadingText || children}
          {getLoadingIcon()}
        </Box>
      );
    }

    return children;
  };

  const getStartIcon = () => {
    if (loading && loadingPosition === 'start') {
      return undefined;
    }
    return startIcon;
  };

  const getEndIcon = () => {
    if (loading && loadingPosition === 'end') {
      return undefined;
    }
    return endIcon;
  };

  return (
    <Button
      {...props}
      disabled={disabled || loading}
      startIcon={getStartIcon()}
      endIcon={getEndIcon()}
      sx={{
        position: 'relative',
        ...props.sx,
      }}
    >
      {getContent()}
    </Button>
  );
};

export default LoadingButton;
