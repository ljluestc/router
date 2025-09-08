import React from 'react';
import {
  Dialog,
  DialogContent,
  Box,
  Typography,
  CircularProgress,
  LinearProgress,
} from '@mui/material';

export type LoadingVariant = 'circular' | 'linear';

interface LoadingDialogProps {
  open: boolean;
  message?: string;
  variant?: LoadingVariant;
  progress?: number;
  size?: number;
  color?: 'primary' | 'secondary' | 'error' | 'warning' | 'info' | 'success';
  thickness?: number;
  showProgress?: boolean;
  maxWidth?: 'xs' | 'sm' | 'md' | 'lg' | 'xl';
  fullWidth?: boolean;
}

const LoadingDialog: React.FC<LoadingDialogProps> = ({
  open,
  message = 'Loading...',
  variant = 'circular',
  progress = 0,
  size = 40,
  color = 'primary',
  thickness = 4,
  showProgress = false,
  maxWidth = 'sm',
  fullWidth = true,
}) => {
  const renderContent = () => {
    if (variant === 'linear') {
      return (
        <Box width="100%">
          <Typography variant="body1" color="text.secondary" textAlign="center" mb={2}>
            {message}
          </Typography>
          <LinearProgress
            variant={showProgress ? 'determinate' : 'indeterminate'}
            value={progress}
            color={color}
            thickness={thickness}
            sx={{
              height: thickness,
              borderRadius: thickness / 2,
            }}
          />
          {showProgress && (
            <Typography variant="body2" color="text.secondary" textAlign="center" mt={1}>
              {Math.round(progress)}%
            </Typography>
          )}
        </Box>
      );
    }

    return (
      <Box display="flex" flexDirection="column" alignItems="center" gap={2}>
        <CircularProgress
          size={size}
          color={color}
          thickness={thickness}
          variant={showProgress ? 'determinate' : 'indeterminate'}
          value={progress}
        />
        <Typography variant="body1" color="text.secondary" textAlign="center">
          {message}
        </Typography>
        {showProgress && (
          <Typography variant="body2" color="text.secondary" textAlign="center">
            {Math.round(progress)}%
          </Typography>
        )}
      </Box>
    );
  };

  return (
    <Dialog
      open={open}
      maxWidth={maxWidth}
      fullWidth={fullWidth}
      PaperProps={{
        sx: {
          borderRadius: 2,
          p: 3,
        },
      }}
      disableEscapeKeyDown
      disableBackdropClick
    >
      <DialogContent>
        {renderContent()}
      </DialogContent>
    </Dialog>
  );
};

export default LoadingDialog;
