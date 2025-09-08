import React, { useState } from 'react';
import {
  Button,
  ButtonProps,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  Typography,
  Box,
  IconButton,
} from '@mui/material';
import {
  Close as CloseIcon,
  Warning as WarningIcon,
  Error as ErrorIcon,
  Info as InfoIcon,
  CheckCircle as CheckCircleIcon,
} from '@mui/icons-material';

export type ConfirmSeverity = 'info' | 'warning' | 'error' | 'success';

interface ConfirmButtonProps extends Omit<ButtonProps, 'onClick'> {
  onConfirm: () => void;
  confirmTitle?: string;
  confirmMessage?: string;
  confirmSeverity?: ConfirmSeverity;
  confirmButtonText?: string;
  cancelButtonText?: string;
  requireConfirmation?: boolean;
  onCancel?: () => void;
}

const ConfirmButton: React.FC<ConfirmButtonProps> = ({
  onConfirm,
  confirmTitle = 'Confirm Action',
  confirmMessage = 'Are you sure you want to proceed?',
  confirmSeverity = 'warning',
  confirmButtonText = 'Confirm',
  cancelButtonText = 'Cancel',
  requireConfirmation = true,
  onCancel,
  children,
  ...buttonProps
}) => {
  const [open, setOpen] = useState(false);

  const handleClick = () => {
    if (requireConfirmation) {
      setOpen(true);
    } else {
      onConfirm();
    }
  };

  const handleConfirm = () => {
    onConfirm();
    setOpen(false);
  };

  const handleCancel = () => {
    onCancel?.();
    setOpen(false);
  };

  const getIcon = () => {
    switch (confirmSeverity) {
      case 'info':
        return <InfoIcon color="info" />;
      case 'warning':
        return <WarningIcon color="warning" />;
      case 'error':
        return <ErrorIcon color="error" />;
      case 'success':
        return <CheckCircleIcon color="success" />;
      default:
        return <WarningIcon color="warning" />;
    }
  };

  const getConfirmColor = () => {
    switch (confirmSeverity) {
      case 'info':
        return 'primary';
      case 'warning':
        return 'warning';
      case 'error':
        return 'error';
      case 'success':
        return 'success';
      default:
        return 'primary';
    }
  };

  return (
    <>
      <Button
        {...buttonProps}
        onClick={handleClick}
      >
        {children}
      </Button>

      <Dialog
        open={open}
        onClose={handleCancel}
        maxWidth="sm"
        fullWidth
        PaperProps={{
          sx: {
            borderRadius: 2,
          },
        }}
      >
        <DialogTitle>
          <Box display="flex" alignItems="center" gap={1}>
            {getIcon()}
            <Typography variant="h6" component="span">
              {confirmTitle}
            </Typography>
          </Box>
          <IconButton
            onClick={handleCancel}
            sx={{
              position: 'absolute',
              right: 8,
              top: 8,
            }}
          >
            <CloseIcon />
          </IconButton>
        </DialogTitle>
        <DialogContent>
          <Typography variant="body1" color="text.secondary">
            {confirmMessage}
          </Typography>
        </DialogContent>
        <DialogActions>
          <Button
            onClick={handleCancel}
            variant="outlined"
          >
            {cancelButtonText}
          </Button>
          <Button
            onClick={handleConfirm}
            variant="contained"
            color={getConfirmColor()}
          >
            {confirmButtonText}
          </Button>
        </DialogActions>
      </Dialog>
    </>
  );
};

export default ConfirmButton;
