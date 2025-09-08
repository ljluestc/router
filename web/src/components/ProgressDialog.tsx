import React from 'react';
import {
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  Button,
  Box,
  Typography,
  LinearProgress,
  CircularProgress,
  Stepper,
  Step,
  StepLabel,
  StepContent,
} from '@mui/material';

export type ProgressVariant = 'linear' | 'circular' | 'stepper';

interface ProgressDialogProps {
  open: boolean;
  onClose?: () => void;
  title?: string;
  message?: string;
  variant?: ProgressVariant;
  progress?: number;
  steps?: string[];
  currentStep?: number;
  size?: number;
  color?: 'primary' | 'secondary' | 'error' | 'warning' | 'info' | 'success';
  thickness?: number;
  showProgress?: boolean;
  showCancel?: boolean;
  cancelText?: string;
  maxWidth?: 'xs' | 'sm' | 'md' | 'lg' | 'xl';
  fullWidth?: boolean;
}

const ProgressDialog: React.FC<ProgressDialogProps> = ({
  open,
  onClose,
  title = 'Progress',
  message,
  variant = 'linear',
  progress = 0,
  steps = [],
  currentStep = 0,
  size = 40,
  color = 'primary',
  thickness = 4,
  showProgress = true,
  showCancel = true,
  cancelText = 'Cancel',
  maxWidth = 'sm',
  fullWidth = true,
}) => {
  const renderContent = () => {
    switch (variant) {
      case 'stepper':
        return (
          <Box>
            {message && (
              <Typography variant="body1" color="text.secondary" textAlign="center" mb={2}>
                {message}
              </Typography>
            )}
            <Stepper activeStep={currentStep} orientation="vertical">
              {steps.map((step, index) => (
                <Step key={index}>
                  <StepLabel>{step}</StepLabel>
                  <StepContent>
                    <Typography variant="body2" color="text.secondary">
                      {index === currentStep ? 'In progress...' : index < currentStep ? 'Completed' : 'Pending'}
                    </Typography>
                  </StepContent>
                </Step>
              ))}
            </Stepper>
            {showProgress && (
              <Box mt={2}>
                <LinearProgress
                  variant="determinate"
                  value={progress}
                  color={color}
                  thickness={thickness}
                  sx={{
                    height: thickness,
                    borderRadius: thickness / 2,
                  }}
                />
                <Typography variant="body2" color="text.secondary" textAlign="center" mt={1}>
                  {Math.round(progress)}%
                </Typography>
              </Box>
            )}
          </Box>
        );

      case 'circular':
        return (
          <Box display="flex" flexDirection="column" alignItems="center" gap={2}>
            <CircularProgress
              size={size}
              color={color}
              thickness={thickness}
              variant={showProgress ? 'determinate' : 'indeterminate'}
              value={progress}
            />
            {message && (
              <Typography variant="body1" color="text.secondary" textAlign="center">
                {message}
              </Typography>
            )}
            {showProgress && (
              <Typography variant="body2" color="text.secondary" textAlign="center">
                {Math.round(progress)}%
              </Typography>
            )}
          </Box>
        );

      default: // linear
        return (
          <Box width="100%">
            {message && (
              <Typography variant="body1" color="text.secondary" textAlign="center" mb={2}>
                {message}
              </Typography>
            )}
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
  };

  return (
    <Dialog
      open={open}
      maxWidth={maxWidth}
      fullWidth={fullWidth}
      PaperProps={{
        sx: {
          borderRadius: 2,
        },
      }}
      disableEscapeKeyDown
      disableBackdropClick
    >
      {title && (
        <DialogTitle>
          <Typography variant="h6" component="span">
            {title}
          </Typography>
        </DialogTitle>
      )}
      <DialogContent>
        {renderContent()}
      </DialogContent>
      {showCancel && onClose && (
        <DialogActions>
          <Button onClick={onClose} variant="outlined">
            {cancelText}
          </Button>
        </DialogActions>
      )}
    </Dialog>
  );
};

export default ProgressDialog;
