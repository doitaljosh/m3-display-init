Tesla Model 3 MCU display hacked by Arduino

# Description:

Using an Arduino to initialize a Tesla Model 3 MCU display connected
to a DS90UB949 serializer over I2C.

# Order of operations:

1. Set GPIO parameters on local serializer.
2. Enable I2C passthrough.
3. Set GPIO parameters on remote deserializer.
4. Turn on backlight by setting a remote GPIO high.
5. Enable remote PORT1 I2C slave address.
